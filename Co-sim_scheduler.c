/* The following code operates as coordinator of a co-simulation between Matlab and Riverbed Modeler(formerly Opnet). */
/* Our examined test case consists of : a power system network and a controller both implemented in Matlab(MatPower)  */
/* and the system's telecommunications network implemented in Riverbed Modeler.										  */

/* Co-simulation Concept */
/* --------------------- */
/* 						 */
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <engine.h>	/* Library contains necessary functions definition which enable the usage of matlab engine.	More info in Matlab doc.*/
#include <time.h>
#include <esa.h>	/* Library defines necessary functions, which give access and let developers control a Riverbed simulated network.	*/

#define LOOPS_EXECUTION		96	/* Specify how many loops of the simulation we want to run. Each loop represents 15 simulated minutes.	*/
#define NUMBER_OF_NODES		7	/* Define total number of nodes (Agents) contained in our network repository(including the controller).	*/

/* Create a flag to specify the destination of all packets. */
typedef enum {DATA_TO_CONTROLLER, DATA_TO_AGENTS} direction;

/* The structure we are interested to exchange with ESys(Riverbed Modeler External System) */
typedef struct node
	{
	char						source_name[20];
	char						dest_name[20];
	double						data_payload;
	double						ete_delay;
	char						message[200];
	int							already_sent;
	double						simulated_time_pk_send;
	double						simulated_time_pk_arrived;
	struct node*				next;
	}Node_Data;

/* Pre-define functions to be used in the co-simulation. */
Engine* Matlab_Init(void);
Node_Data* Create_Network_Node_List(direction );
double Calculate_Average_RTDelay(Node_Data *, Node_Data *);
int Matlab_Agents_to_Opnet(Engine *, Node_Data *);
int Opnet_to_Matlab_Controller(Engine *, Node_Data *);
int Update_Matlab(Engine *, double );
int Finalize_Simulation(Engine *);
double Riverbed_Network_simulated_time(Node_Data *);
void Free_Alloc_Mem(Node_Data *);
void Esa_Callback_Function(void *state, double time, void *data); /* I didn't have to use it. Just left it in the code to help future developers. Read Riverbed Documentation for further details. */
/*-------------------------------------------------------*/

int main(int argc, char **argv)
{
	EsaT_State_Handle	state_handler;	/* Associate a state handler to let us access Riverbed ESys module using ESA specific functions.	*/
	EsaT_Interface		*interface_ptr;	/* A pointer to all ESYS interfaces in our network.													*/
	EsaT_Interface		data_intrf;		/* A variable to hold information about ONE specific ESYS interface.								*/
    int					interfaces_num;	/* Holds the total number of existing Riverbed interfaces.											*/
	int					status;			/* Check whether there is an error inside Riverbed or not.											*/
	double				return_time;	/* SIMULATED TIME at which Riverbed returned control to C code.										*/
	int					event_num;		/* Number of event at which Riverbed returned control to C code.									*/
	Engine				*mat_ptr;		/* Create a pointer variable to store Matlab engine address for later use.							*/
    Node_Data			*ag_head;		/* Create a pointer variable to store the linked list containing all data of exchanging interest.	*/
	Node_Data			*control_header;/* Same as *ag_head, but this time for linked list with data from controller to agents.				*/
	int					i; 				/* Create an integer variable to use in 'for' loop.													*/
	clock_t				sim_start_clock_time = clock(); /* Store simulation starting time in sim_start_clock_time variable.					*/
	clock_t				loop_start_clock_time, power_flow_start_clock_time, controller_start_clock_time, comms_to_controller_clock_start, comms_to_agents_clock_start;
	double				end_loop_time[LOOPS_EXECUTION], end_power_flow_clock_time[LOOPS_EXECUTION], end_controller_clock_time[LOOPS_EXECUTION];
	time_t				before_com, after_com, sim_start_calendar_time;
	double				mid_com_diff_t, mid_comms_clock_time, total_com_diff_t[LOOPS_EXECUTION], total_comms_diff_clock_time[LOOPS_EXECUTION], total_comms_diff_network_time[LOOPS_EXECUTION], agents_to_controller_network_time[LOOPS_EXECUTION], controller_to_agents_network_time[LOOPS_EXECUTION], global_RTD_array[LOOPS_EXECUTION];
	double				runtime_diff_ms; /* Create a variable to calculate ellapsed time.													*/
	double				head_delay[LOOPS_EXECUTION];
	FILE 				*file_ptr = fopen("5_Agent_Simulation_Duration.csv", "w");
	
	/* We are interested not only in the co-simulation results, but also in time elapsed in each simulator.									*/
	/* That's is why our main function is full of clock_t and time variables.																*/
	/* I personally prefer clock_t to calculate ellapsed time, as it is closely related to cpu clock,										*/
	/* but i also included time_t in some cases so devellopers can later choose the method they prefer.										*/
	
	if (file_ptr == NULL)
	{
		printf("Error : Couldn't open file for data input!\n");
		exit(1);
	}
	time(&sim_start_calendar_time);
	/* Starts Matlab Engine and initialize Matlab Side of the co-simulation. For more information about Matlab Engine check Matlab Docs.	*/
	mat_ptr = Matlab_Init();
	ag_head = Create_Network_Node_List(DATA_TO_CONTROLLER);	/* Create a linked list containing data agents have to send to the controller. Function returns a pointer to the head of the list which is copied later to ag_head.																					 */
	control_header = Create_Network_Node_List(DATA_TO_AGENTS); /* Does the same as before but in this case, data of interest are data sent from controller to agents.	*/
	
	/* Starts Riverbed Modeler and initializes Riverbed Side of the co-simulation. */
	Esa_Main(argc, argv, ESAC_OPTS_NONE);
    Esa_Init(argc, argv, ESAC_OPTS_NONE, &state_handler);
    Esa_Load(state_handler, ESAC_OPTS_NONE);
    Esa_Interface_Group_Get(state_handler, &interface_ptr, &interfaces_num); /* Retrieves the group of ALL Riverbed ESYS interfaces			*/
	data_intrf = interface_ptr[0]; /* Since there ONLY ONE interface in our network there is no need to iterate through the group to find the corect interface, as we should do if we had more than one interfaces, or even more complicated more than one ESYS modules.									  */
	Esa_Interface_Callback_Register(state_handler, &status, data_intrf, Esa_Callback_Function, 0, 0); /* Register a Callback function (in this case Esa_Callback_Function) to a specific interface (in this case data_intrf) to call every time Riverbed side interrupts C side of the co-simulation.	*/
	
	for( i = 1; i <= LOOPS_EXECUTION; i++ ) /* Get in the simulation loop for all Simulators.												*/
	{
		loop_start_clock_time = clock();
		switch(i)
		{
			case 1 :
			printf("\n%dst loop of co-simulation\n", i);
			break;
			case 2 :
			printf("\n%dnd loop of co-simulation\n", i);
			break;
			case 3 :
			printf("\n%drd loop of co-simulation\n", i);
			break;
			default :
			printf("\n%dth loop of co-simulation\n", i);
		}
		/* Metering data at power system nodes and retrieve data from Agents. Data for each agent associated with the correct agent in our linked list.				*/
		power_flow_start_clock_time = clock();
		Matlab_Agents_to_Opnet(mat_ptr, ag_head);
		end_power_flow_clock_time[i-1] = ((double)clock() - power_flow_start_clock_time)/CLOCKS_PER_SEC;
		
		/* Sending data collected from Matlab Agents to Controller through Riverbed's telecommunication network in order to gather telecommunication information.	*/
		time(&before_com);
		comms_to_controller_clock_start = clock();
		Esa_Interface_Value_Set(state_handler, &status, data_intrf, ESAC_NOTIFY_IMMEDIATELY, ag_head); /* Make data available in Riverbed by associating the pointer to the head of agents linked list to data_intrf ESYS interface IMMEDIATELY.																					*/
		Esa_Execute_Until(state_handler, &status, (15*60*(i-0.5)), ESAC_UNTIL_INCLUSIVE, &return_time, &event_num); /* Hand over control to Riverbed to simulate it's network for a specified time.																															 	  */
		printf("\n------Returned from Agents------\n");
		/* Retrieving data from Riverbed, in order to make them available for the co-simulation. Since we exchange data by reference we don't need to do this, but it's good as general idea. I left this in the code to help future developers.																					*/
		Esa_Interface_Value_Get(state_handler, &status, data_intrf, &ag_head);
		agents_to_controller_network_time[i-1] = Riverbed_Network_simulated_time(ag_head);
		time(&after_com);
		mid_com_diff_t = difftime(after_com, before_com);
		mid_comms_clock_time = ((double)clock() - comms_to_controller_clock_start)/CLOCKS_PER_SEC;
		
		/* Run Controller's optimization algorithm and assign data of interest to associated Agents in our created linked list.										*/
		controller_start_clock_time = clock();
		Opnet_to_Matlab_Controller(mat_ptr, control_header);
		end_controller_clock_time[i-1] = ((double)clock() - controller_start_clock_time)/CLOCKS_PER_SEC;
		
		/* Send new setpoints back to Agents from Controller through Riverbed's network.																			*/
		time(&before_com);
		comms_to_agents_clock_start = clock();
		Esa_Interface_Value_Set(state_handler, &status, data_intrf, ESAC_NOTIFY_IMMEDIATELY, control_header);
		Esa_Execute_Until(state_handler, &status, (15*60*i), ESAC_UNTIL_INCLUSIVE, &return_time, &event_num);
		printf("\n------Returned from Controller------\n");
		Esa_Interface_Value_Get(state_handler, &status, data_intrf, &control_header);
		controller_to_agents_network_time[i-1] = Riverbed_Network_simulated_time(control_header);
		//Esa_Current_Event_Time_Set (state_handler, (15*60*i));
		time(&after_com);
		total_comms_diff_clock_time[i-1] = mid_comms_clock_time + ((double)clock() - comms_to_agents_clock_start)/CLOCKS_PER_SEC;
		total_com_diff_t[i-1] = mid_com_diff_t + difftime(after_com, before_com);
		/* Assign the total communication time needed for by the system to send all the packets to and from the controller. */
		total_comms_diff_network_time[i-1] = Riverbed_Network_simulated_time(ag_head) + Riverbed_Network_simulated_time(control_header);
		/* Global average round-trip delay calculation for this iteration. */
		global_RTD_array[i-1] = Calculate_Average_RTDelay(ag_head, control_header);
		/* Update Power System Data in Matlab */
		head_delay[i-1] = ag_head->ete_delay + control_header->ete_delay; /* This variable is not really important. I just use this to plot the RTD(round trip delay) of the first node communication with the Controller. It helps to get a visual idea of the way the communication network works.										*/
		Update_Matlab(mat_ptr, head_delay[i-1]);
		end_loop_time[i-1] = ((double)clock() - loop_start_clock_time)/CLOCKS_PER_SEC;
	}
	/* Write data regarding simulation execution time in a file of csv format, so they can be available for later use.												*/
	fprintf(file_ptr,"Loop_Execution_Time, Power_Flow_Execution_Time, Controller_Execution_Time, Total_Communication_Simulation_Execution_Time, Total_Communication_Network_Time, Agents_to_Controller_Network_Time, Controller_to_Agents_Network_Time, Global_Average_Delay, Agent1_Controller_RTD\n");
	for( i = 1; i <= LOOPS_EXECUTION; i++ )
	{
		fprintf(file_ptr, "%.4f,%.4f,%.4f,%.4f, %.4f, %.4f, %.4f, %.4f, %.4f\n", end_loop_time[i-1], end_power_flow_clock_time[i-1], end_controller_clock_time[i-1], total_comms_diff_clock_time[i-1], total_comms_diff_network_time[i-1], agents_to_controller_network_time[i-1], controller_to_agents_network_time[i-1], global_RTD_array[i-1], head_delay[i-1]); 
	}
	
	/* Terminate Riverbed Modeler Simulator. */
	Esa_Terminate (state_handler, ESAC_TERMINATE_NORMAL);
	//Calculate_Average_RTDelay(ag_head, control_header);
	//getchar(); /* Just wait for input before closing matlab Engine, so that we can observe the head delay plot figure.											*/
	
	/* Finalize simulation by closing matlab engine.																												*/
	Finalize_Simulation(mat_ptr);
	Free_Alloc_Mem(ag_head); /* Free up allocated memmory.																											*/
	Free_Alloc_Mem(control_header);
	/*--------------------------------------------------------------------------*/
	runtime_diff_ms = ((double)clock() - sim_start_clock_time)/CLOCKS_PER_SEC;
	printf("\nSimulation took %.2f seconds to complete...", runtime_diff_ms);
	printf("Simulation took %.2f wall time seconds to complete...\n", (double)(time(NULL) - sim_start_calendar_time));
	fprintf(file_ptr,"\n\n\nTotal_Clock_Simulation_Time,%.2f\nTotal_wall_Simulation_Time,%.2f\n", runtime_diff_ms, (double)(time(NULL) - sim_start_calendar_time));
	fclose(file_ptr);
	/*--------------------------------------------------------------------------*/
	//getchar();
	_CrtDumpMemoryLeaks();
	return 0;
}

void Esa_Callback_Function(void *state, double time, void *data)
{
	//printf("\nCame here because of Riverbed Callback.\n");
	Esa_Interrupt(state);
}

Engine* Matlab_Init()
{
	Engine *ep;
	
	/* Start Matlab Engine */
	if (!(ep = engOpen(""))) {
		fprintf(stderr, "\nCan't start MATLAB engine\n");
		return NULL;
	}
	
	/* Initialize Matlab Side for Co-simulation by writting matlab commands through C code.																			*/
	engEvalString(ep, "addpath('C:\\Users\\sl\\Documents\\MATLAB\\matpower5.1')");
	engEvalString(ep, "addpath('C:\\Users\\sl\\Documents\\MATLAB\\Full_5bus_LV_CVC')");
	engEvalString(ep, "Matlab_Power_System_Initialization");
	
	return ep; /* Return Matlab Engine pointer to main so that it is possible to access the Engine later in the simulation flow.									*/
}


Node_Data* Create_Network_Node_List(direction direction_flag)
{
	Node_Data *header;
	Node_Data *temp_ptr;
	/* The following array needs to contain name of string (like Agent1,etc) equal to the NUMBER_OF_NODES in the examined network.									*/
	char *Agents[NUMBER_OF_NODES] = {"Agent1", "Agent2", "Agent3", "Agent4", "Agent5", "Agent_transf", "Controller"};
	int i=0;

	header = malloc( sizeof(Node_Data) );
	if (direction_flag == DATA_TO_CONTROLLER)
	{
		sprintf(header->source_name,"%s",Agents[0]);
		sprintf(header->dest_name,"%s",Agents[NUMBER_OF_NODES-1]);
	}
	else if (direction_flag == DATA_TO_AGENTS)
	{
		sprintf(header->source_name,"%s",Agents[NUMBER_OF_NODES-1]);
		sprintf(header->dest_name,"%s",Agents[0]);
	}
	else
	{
		fprintf(stderr, "You have entered invalid enum value...\n");
		exit(1);
	}
	header->next = NULL;
	temp_ptr = header;
    if ( temp_ptr != NULL ) {
        while ( temp_ptr->next != NULL)
        {
            temp_ptr = temp_ptr->next;
        }
    }
	// Add nodes at the end of the list
	for (i=1; i<=NUMBER_OF_NODES-2; i++)
	{
		temp_ptr->next = malloc( sizeof(Node_Data) );
		temp_ptr = temp_ptr->next;

		if ( temp_ptr == 0 )
		{
			printf( "Out of memory\n" );
			return NULL;
		}
		// initialize new memory
		
		temp_ptr->next = NULL;
		temp_ptr->data_payload = 1.00;
		temp_ptr->ete_delay = 0.00;
		temp_ptr->already_sent = 1;
		
		if (direction_flag == DATA_TO_CONTROLLER)
			{
				sprintf(temp_ptr->source_name,"%s",Agents[i]);
				sprintf(temp_ptr->dest_name,"%s",Agents[NUMBER_OF_NODES-1]);
			}
		else if (direction_flag == DATA_TO_AGENTS)
			{
				sprintf(temp_ptr->source_name,"%s", Agents[NUMBER_OF_NODES-1]);
				sprintf(temp_ptr->dest_name,"%s",Agents[i]);
			}
		else
			{
			fprintf(stderr, "You have entered invalid enum value...\n");
			exit(1);
			}
	}
	return header;
}

double Calculate_Average_RTDelay(Node_Data *print_agents_head, Node_Data *print_control_head)
{
	Node_Data *agents_temp_ptr, *control_temp_ptr;
	double total_delay[NUMBER_OF_NODES-1];
	double sum = 0.00;
	double average;
	int index = 0;
	
	agents_temp_ptr = print_agents_head;
	control_temp_ptr = print_control_head;
	
	while ((agents_temp_ptr != 0) && (control_temp_ptr != 0) && (index < NUMBER_OF_NODES-1))
	{
		total_delay[index] = agents_temp_ptr->ete_delay + control_temp_ptr->ete_delay;
		sum += total_delay[index];		
		//printf("\n--------------------\nSource node : %s\nTotal Delay : %f\nAgent Delay : %f\nController Delay : %f\n--------------------\n", agents_temp_ptr->source_name, total_delay[index], agents_temp_ptr->ete_delay, control_temp_ptr->ete_delay);
		/* printf("\n--------------------\nSource node : %s\nDestination node : %s\nDelay = %f\nPayload = %f\n--------------------\n", agents_temp_ptr->source_name, agents_temp_ptr->dest_name, agents_temp_ptr->ete_delay, agents_temp_ptr->data_payload); */
		index++;
		agents_temp_ptr = agents_temp_ptr->next;
		control_temp_ptr = control_temp_ptr->next;
	}
	average = sum / (NUMBER_OF_NODES-1);
	return average;
}

/* A function to compute total time elapsed between the first created packet by the first node and the last received packet by the last node(end of all packets send). */
double Riverbed_Network_simulated_time(Node_Data *list_head)
{
	double network_simulated_time, start_time, end_time;
	Node_Data *list_temp_ptr;
	
	/* Go to the head of the list. */
	list_temp_ptr = list_head;
	start_time = list_temp_ptr->simulated_time_pk_send;
	end_time = list_temp_ptr->simulated_time_pk_arrived;
	
	while (list_temp_ptr != NULL)
	{
		if(list_temp_ptr->simulated_time_pk_send <= start_time)
			start_time = list_temp_ptr->simulated_time_pk_send;
		
		if(list_temp_ptr->simulated_time_pk_arrived >= end_time)
			end_time = list_temp_ptr->simulated_time_pk_arrived;
		
		list_temp_ptr = list_temp_ptr->next;
	}
	
	network_simulated_time = end_time - start_time;
	
	return network_simulated_time;
}

/* A function to calculate average delay. */

/* Metering data at power system nodes and retrieve data from Agents. Data for each agent associated with the correct agent in our linked list.		*/
int Matlab_Agents_to_Opnet(Engine *ep, Node_Data *agents_header)
{
	Node_Data *temp_ptr;
	mxArray *ptr = NULL;
	double *mat_array = NULL;
	int i;
	
	temp_ptr = agents_header;
	engEvalString(ep, "Agents_to_Contoller"); /* Run matlab script "Agents_to_Contoller" using this function (defined in engine.h).					*/
	ptr = engGetVariable(ep, "agents_memory_needs"); /* Retrieve an array from Matlab and associates a mxArray pointer to it.						*/
	mat_array = mxGetPr(ptr);
	for (i=0; i<NUMBER_OF_NODES-1; i++) /* Assign data_payload (size of the packet to send through Riverbed Modeler) to every Agent of our created linked list.	*/
	{
		temp_ptr->data_payload = *(mat_array+i);
		temp_ptr = temp_ptr->next;
	}
	return EXIT_SUCCESS;
}

/* Similar to "Matlab_Agents_to_Opnet". In this case Matlab command to run is the Controller script and not the power flow. Also all data_payloads get written to Agents of Controller's linked list.					*/
int Opnet_to_Matlab_Controller(Engine *ep, Node_Data *controller_header)
{	
	Node_Data *temp_ptr;
	mxArray *ptr = NULL;
	double *mat_array = NULL;
	int i;
	
	temp_ptr = controller_header;
	engEvalString(ep, "Controller_to_Agents");
	ptr = engGetVariable(ep, "controller_memory_needs");
	mat_array = mxGetPr(ptr);
	
	for (i=0; i<NUMBER_OF_NODES-1; i++)
	{
		temp_ptr->data_payload = *(mat_array+i);
		temp_ptr = temp_ptr->next;
	}
	return EXIT_SUCCESS;
}

/* In the last step of EACH loop, power system needs to update it's current state. */
int Update_Matlab(Engine *ep, double total_delay)
{
	mxArray *data_ptr = NULL;
	
	data_ptr = mxCreateDoubleMatrix(1, 1, mxREAL);
	*mxGetPr(data_ptr) = total_delay;
	//or use
	//data_ptr = mxCreateDoubleScalar(total_delay);
	//insted to get the same result.
	printf("\nUpdate Head Total Delay = %f\n", total_delay);
	engPutVariable(ep, "mat_delay", data_ptr);
	engEvalString(ep, "x_axis = x_axis + 1;");
	engEvalString(ep, "addpoints(h,x_axis,mat_delay);");
	engEvalString(ep, "drawnow");
	
	engEvalString(ep, "Power_System_Update");
	
	return EXIT_SUCCESS;
}

/* Finalizing Matlab Side Simulation Sequence. Calls Matlab Engine and executes two command. The last command "close" closes matlab engine. */
int Finalize_Simulation(Engine *ep)
{
	engEvalString(ep, "Mean_loss=sum(Ploss)/96;");
	engEvalString(ep, "close;");
	engClose(ep);

	return EXIT_SUCCESS;
}

/* Free dynamic allocated Memory, since we reached the end of the simulation. */
void Free_Alloc_Mem(Node_Data *free_head)
{
	Node_Data *temp_ptr;
	
	while (free_head != 0)
	{
		temp_ptr = free_head;
		free_head = free_head->next;
		free(temp_ptr);
	}
}