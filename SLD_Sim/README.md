#### Create SLD simulation
File Structure:
SimulationMocks.h: Mocks for hardware, threading, observers, and logging.
TxtSortingLine.h: Header for the sorting line class and states.
TxtSortingLineCalibData.cpp: Mocked calibration data loading/saving.
TxtSortingLine.cpp: Core logic and constructor.
TxtSortingLineRun.cpp: Finite State Machine (FSM) implementation.
main.cpp: Entry point to drive the simulation with inputs (White/Red/Blue).

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
