#### Create VGR simulation
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
