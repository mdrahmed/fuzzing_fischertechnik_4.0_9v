#### Create HBW simulation
File Structure:
1. SimulationMocks.h: Mocks for hardware, threading, observers, logging, and common types.
2. TxtHighBayWarehouseStorage.h: Header for inventory management.
3. TxtHighBayWarehouseStorage.cpp: Implementation of inventory logic (with print functionality).
4. TxtHighBayWarehouse.h: Header for the main HBW robot class.
5. TxtHighBayWarehouseCalibData.cpp: Calibration data loading/saving.
6. TxtHighBayWarehouse.cpp: Core robot logic and constructor.
7. TxtHighBayWarehouseRun.cpp: Finite State Machine (FSM) implementation.
8. main.cpp: Entry point to drive the simulation interactively.

Run:
```
g++ -std=c++11 -pthread main.cpp TxtHighBayWarehouse.cpp TxtHighBayWarehouseRun.cpp TxtHighBayWarehouseStorage.cpp TxtHighBayWarehouseCalibData.cpp -o hbw_sim
```
Input: fetch/store white/red/blue

#### FUZZ HBW simulation
```
g++ -std=c++11 -pthread fuzz_hbw.cpp TxtHighBayWarehouse.cpp TxtHighBayWarehouseRun.cpp TxtHighBayWarehouseStorage.cpp TxtHighBayWarehouseCalibData.cpp -o fuzz_hbw
```
