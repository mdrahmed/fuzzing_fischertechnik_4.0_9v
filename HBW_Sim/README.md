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
./hbw_sim
```
Input: fetch/store white/red/blue

#### FUZZ HBW simulation
```
g++ -std=c++11 -pthread fuzz_hbw.cpp TxtHighBayWarehouse.cpp TxtHighBayWarehouseRun.cpp TxtHighBayWarehouseStorage.cpp TxtHighBayWarehouseCalibData.cpp -o fuzz_hbw
```

##### Attacks Overview
Details of all the attacks can be found following,
```
// Covers Attack 1. High Bay-Warehouse storage (Collision)
attack_StorageCollision();      

// Covers Attack 2. High Bay-Warehouse storage (Underflow)
attack_StorageUnderflow();     

// Covers Attack 3. High Bay-Warehouse moveCR - (Crash)
attack_InvalidCoordinates();   

// Covers Attacks 4, 5, 6, 7. High Bay-Warehouse (Crash/Deadlock)
attack_StateFlooding();         

// Covers Attacks 8,10. High Bay-Warehouse (Misconfiguration), 9.High Bay-Warehouse (Joystick collision with storage), 
attack_JoystickMisconfig();     

// Covers Attack 11. High Bay-Warehouse ( stall/jam)
attack_AxisRace();              

// Covers Attacks 12. High Bay-Warehouse (Deadlock), 13. High Bay-Warehouse (Undefined behaviour)
attack_MqttTimeout();           

// Covers Attack 14. High Bay-Warehouse VGR - storage lost
attack_ProcessStorageRace();    

// Covers Attacks 15, 16. High Bay-Warehouse  & Vacuum-Gripper Robot 
attack_ConveyorJam();           
```