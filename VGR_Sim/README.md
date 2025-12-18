#### Create VGR simulation
File Structure:
1. SimulationMocks.h: Mocks for hardware, threading, observers, logging, and common types.
2. TxtVacuumGripper.h: Header for the vacuum gripper actuator.
3. TxtVacuumGripper.cpp: Implementation of the vacuum gripper logic.
4. TxtNfcDevice.h: Header for the NFC reader.
5. TxtNfcDevice.cpp: Implementation of the NFC reader.
6. TxtVacuumGripperRobot.h: Header for the main robot controller.
7. TxtVacuumGripperRobotCalibData.cpp: Calibration data handling.
8. TxtVacuumGripperRobotRun.cpp: State machine implementation.
9. TxtVacuumGripperRobot.cpp: Main robot logic and movement sequences.
10. main.cpp: Entry point with CLI interface and simulated storage display.

Run:
```
g++ -std=c++11 -pthread main.cpp TxtVacuumGripperRobot.cpp TxtVacuumGripperRobotRun.cpp TxtVacuumGripperRobotCalibData.cpp TxtVacuumGripper.cpp TxtNfcDevice.cpp -o vgr_sim
./vgr_sim
```
At first, the storage matrix will be printed. Then type following commands to fetch/store workpieces.
```
store red
store blue
fetch red
store white
fetch white
quit
```

#### Fuzz VGR simulation
```
g++ -std=c++11 -pthread fuzz_vgr.cpp TxtVacuumGripperRobot.cpp TxtVacuumGripperRobotRun.cpp TxtVacuumGripperRobotCalibData.cpp TxtVacuumGripper.cpp TxtNfcDevice.cpp -o fuzz_vgr
./fuzz_vgr
```
