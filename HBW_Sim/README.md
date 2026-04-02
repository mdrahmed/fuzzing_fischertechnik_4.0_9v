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

Compile the fuzzer:
```
afl-clang-fast++ -fsanitize=address,undefined -O1 -g \
  fuzz_hbw_afl.cpp \
  TxtHighBayWarehouse.cpp \
  TxtHighBayWarehouseRun.cpp \
  TxtHighBayWarehouseStorage.cpp \
  TxtHighBayWarehouseCalibData.cpp \
  -I. -o fuzz_hbw
```

Create seeds:
```
mkdir -p seeds
python3 gen_seeds.py

afl-fuzz -i seeds/ -o findings/ -- ./fuzz_hbw
```
