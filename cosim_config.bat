:: Make your life easier. Avoid entering the same commands again and again.
@color 0f
@cd/users/sl/op_models
::@cl /c matop.c
::@cl /c matop10ag.c
::@cl /c matop20ag.c

@echo -------------------------------------
@echo Press ctr+c to stop running sequence.
@pause
::-------ETHCOAX-------
::@op_mksim -net_name cosim-final_ethcoax_cosim -c -raw_err TRUE
::@op_mksim -net_name cosim-final_ethcoax_cosim_10ag -c -raw_err TRUE
::@op_mksim -net_name cosim-final_ethcoax_no_cosim_10ag -c -raw_err TRUE
::@op_mksim -net_name cosim-final_ethcoax_cosim_20ag -c -raw_err TRUE
::-------ETHERNET------
::@op_mksim -net_name cosim-final_ethernet_cosim -c
::@op_mksim -net_name cosim-final_ethernet_cosim_10ag -c
::@op_mksim -net_name cosim-final_ethernet_no_cosim_EIGRP -c
::@op_mksim -net_name cosim-final_ethernet_cosim_20ag -c
::------WLAN WIFI------
::@op_mksim -net_name cosim-wlan_cosim -c
::@op_mksim -net_name cosim-wlan_cosim_v2 -c
::@op_mksim -net_name cosim-wlan_cosim_10ag -c
::@op_mksim -net_name cosim-wlan_cosim_20ag -c
::-------MANET---------
::@op_mksim -net_name cosim-manets_built_no_cosim -c
::@op_mksim -net_name cosim-manets_built_cosim -c
::@op_mksim -net_name cosim-manets_built_cosim_10ag -c
::@op_mksim -net_name cosim-manets_built_cosim_20ag -c
::-------SMART_MAC-----
::@op_mksim -net_name cosim-smart_mac_built_cosim_7 -c
::-------WIMAX---------
::@cosim.project\cosim-smart_mac_built_cosim_7.dev32.i0.sim -duration 86400 -noprompt -ef cosim-smart_mac_built_cosim_7-DES-1

@echo -------------------------------------
@echo Press ctr+c to stop running sequence. Everything is compiled and linked. Just run all 12 models once (Ethcoax, Ethernet, Wifi, Manet).
@pause
@cls
::-------ETHCOAX-------
::@cosim.project\cosim-final_ethcoax_cosim.dev32.i0.sim -ef cosim-final_ethcoax_cosim-DES-1 -noprompt                     
::@cosim.project\cosim-final_ethcoax_cosim_10ag.dev32.i0.sim -ef cosim-final_ethcoax_cosim_10ag-DES-1 -noprompt
::@cosim.project\cosim-final_ethcoax_no_cosim_10ag.dev32.i0.sim -duration 100 -noprompt -debug -diag_enable -endsim_memstats > test_results.txt
::@cosim.project\cosim-final_ethcoax_cosim_20ag.dev32.i0.sim -ef cosim-final_ethcoax_cosim_20ag-DES-1 -noprompt
::-------ETHERNET------
::@cosim.project\cosim-final_ethernet_cosim.dev32.i0.sim -ef cosim-final_ethernet_cosim-DES-1 -noprompt
::@cosim.project\cosim-final_ethernet_cosim_10ag.dev32.i0.sim -ef cosim-final_ethernet_cosim_10ag-DES-1 -noprompt
::@cosim.project\cosim-final_ethernet_no_cosim_EIGRP.dev32.i0.sim -ef cosim-final_ethcoax_no_cosim_10ag_original-DES-1 -noprompt -debug
::@cosim.project\cosim-final_ethernet_cosim_20ag.dev32.i0.sim -ef cosim-final_ethernet_cosim_20ag-DES-1 -noprompt
::------WLAN WIFI------
::@cosim.project\cosim-wlan_cosim.dev32.i0.sim -ef cosim-wlan_cosim-DES-1 -noprompt
::@cosim.project\cosim-wlan_cosim_v2.dev32.i0.sim -ef cosim-wlan_cosim_v2-DES-1 -noprompt
::@cosim.project\cosim-wlan_cosim_10ag.dev32.i0.sim -ef cosim-wlan_cosim_10ag-DES-1 -noprompt
::@cosim.project\cosim-wlan_cosim_20ag.dev32.i0.sim -ef cosim-wlan_cosim_20ag-DES-1 -noprompt
::-------MANET---------
::@cosim.project\cosim-manets_built_no_cosim.dev32.i0.sim -duration 86400 -noprompt -ef cosim-manets_built_no_cosim-DES-1
::@cosim.project\cosim-manets_built_cosim.dev32.i0.sim -duration 86400 -noprompt -ef cosim-manets_built_cosim-DES-1
::@cosim.project\cosim-manets_built_cosim_10ag.dev32.i0.sim -duration 86400 -noprompt -ef cosim-manets_built_cosim_10ag-DES-1
::@cosim.project\cosim-manets_built_cosim_20ag.dev32.i0.sim -duration 86400 -noprompt -ef cosim-manets_built_cosim_20ag-DES-1
::-------SMART_MAC-----
::@cosim.project\cosim-smart_mac_built_cosim_7.dev32.i0.sim -duration 86400 -noprompt -ef cosim-smart_mac_built_cosim_7-DES-1
::-------WIMAX---------
::@cosim.project\cosim-smart_mac_built_cosim_7.dev32.i0.sim -duration 86400 -noprompt -ef cosim-smart_mac_built_cosim_7-DES-1

@pause
::@cmd