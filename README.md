# Matlab-Opnet-Co-Simulation
 A co-simulation of Matlab and Opnet ICT Modeler

This project can be utilized as a guide for those who want to exeute and interact with matlab scripts using the **Matlab Engine library**.
The same benefits apply to those who want to interact with Opnet through its **ESA** library.
All interaction are achieved in **pure C**.

The code in `Co-Sim_Scheduler.c` operates as coordinator of a co-simulation between Matlab and Riverbed Modeler(formerly Opnet).
Our examined test case consists of : a power system network and a controller both implemented in Matlab (MatPower)
and the system's telecommunications network implemented in Opnet Modeler (now Riverbed Modeler).

### P.S.
This project only provides the code of the scheduler,`Co-Sim_Scheduler.c`, and the Windows Batch file, `Cosim_config` to automate the testing process.
The models-scripts for both Matlab and Opnet are not provided whatsoever.
