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


##### Attacks Overview
Details of all the attacks can be found following,
```
attack_Collision_MultipleCounts();  // Covers Attack 17
attack_LostColor_Race();            // Covers Attacks 18, 19, 20
attack_CounterOverflow();           // Covers Attack 22
attack_Misclassification();         // Covers Attack 23
attack_BlockingActuator();          // Covers Attack 21
attack_ResourceExhaustion();        // Covers Attacks 25, 29, 30
attack_ConfigFuzzing();             // Covers Attacks 26, 27, 28
attack_AxisUnderflow();             // Covers Attack 31
```
