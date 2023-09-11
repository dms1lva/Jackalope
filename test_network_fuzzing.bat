Z:\Jackalope\build\Release\fuzzer.exe -save_hangs -in C:\Users\Administrator\Documents\fuzz_test_network\inputs ^
-out C:\Users\Administrator\Documents\fuzz_test_network\out -delivery network -iterations 99999 -t1 10000 -t 10001 ^
-address 127.0.0.1 -port 27015 -network_init_time 5000 ^
-instrument_module test_netmode.exe -target_module test_netmode.exe -target_method process_data -cmp_coverage -persist -- Z:\Jackalope\build\Debug\test_netmode.exe 
@REM -instrument_module test_netmode.exe -persist -- Z:\Jackalope\build\Release\test_netmode.exe 
@REM -address 127.0.0.1 -port 27015 -network_init_time 5000 -trace_debug_events -trace_basic_blocks -full_address_map ^