#### Create SLD simulation
File Structure:
1. SimulationMocks.h: Mocks for hardware, threading, observers, and logging.
2. TxtSortingLine.h: Header for the sorting line class and states.
3. TxtSortingLineCalibData.cpp: Mocked calibration data loading/saving.
4. TxtSortingLine.cpp: Core logic and constructor.
5. TxtSortingLineRun.cpp: Finite State Machine (FSM) implementation.
6. main.cpp: Entry point to drive the simulation with inputs (White/Red/Blue).

Run:
```
g++ -std=c++11 -pthread main.cpp TxtSortingLine.cpp TxtSortingLineRun.cpp TxtSortingLineCalibData.cpp -o sld_sim
./sld_sim
```

#### Fuzz the SLD programs
```
g++ -std=c++11 -pthread fuzz_sld.cpp TxtSortingLine.cpp TxtSortingLineRun.cpp TxtSortingLineCalibData.cpp -o fuzz_sld
./fuzz_sld
```
