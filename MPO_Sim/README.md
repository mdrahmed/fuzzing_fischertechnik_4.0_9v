#### Create MPO simulation
File Structure:
1. SimulationMocks.h: Mocks for hardware, threading, observers, and logging.
2. TxtMultiProcessingStation.h: Header for the MPO class and states.
3. TxtMultiProcessingStationCalibData.cpp: Mocked calibration data loading/saving.
4. TxtMultiProcessingStation.cpp: Core logic and constructor.
5. TxtMultiProcessingStationRun.cpp: Finite State Machine (FSM) implementation.
6. main.cpp: Entry point to drive the simulation.

Run:
```
g++ -std=c++11 -pthread main.cpp TxtMultiProcessingStation.cpp TxtMultiProcessingStationRun.cpp TxtMultiProcessingStationCalibData.cpp -o mpo_sim
```
Input: white/red/blue

#### Fuzz MPO simulation
```
g++ -std=c++11 -pthread fuzz_mpo.cpp TxtMultiProcessingStation.cpp TxtMultiProcessingStationRun.cpp TxtMultiProcessingStationCalibData.cpp -o fuzz_mpo
./fuzz_mpo
```
