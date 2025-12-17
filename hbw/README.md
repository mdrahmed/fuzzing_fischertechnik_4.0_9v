####  Create HBW simulation
```
g++ -std=c++11 main.cpp TxtHighBayWarehouse.cpp TxtHighBayWarehouseStorage.cpp -o hbw_sim
./hbw_sim
```
An array is used for storage.

#### Fuzz the HBW programs
```
g++ -std=c++11 -pthread fuzz_hbw.cpp TxtHighBayWarehouse.cpp TxtHighBayWarehouseStorage.cpp -o fuzz_hbw
./fuzz_hbw
```
