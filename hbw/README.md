####  Create HBW simulation
The `TxtHighBayWarehouse.cpp` and `TxtHighBayWarehouseRun.cpp` has been merged. The `main.cpp` is used with other required files to create the simulation.
```
g++ -std=c++11 main.cpp TxtHighBayWarehouse.cpp TxtHighBayWarehouseStorage.cpp -o hbw_sim
./hbw_sim
```
An array is used for storage.

#### Fuzz the HBW programs
The `fuzz_hbw.cpp` will start fuzzing HBW simulation to test all the attacks.
```
g++ -std=c++11 -pthread fuzz_hbw.cpp TxtHighBayWarehouse.cpp TxtHighBayWarehouseStorage.cpp -o fuzz_hbw
./fuzz_hbw
```
