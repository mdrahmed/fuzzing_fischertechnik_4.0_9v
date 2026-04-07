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

### AFL++
Compile the hbw fuzzer:
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
python3 gen_seeds_hbw.py

afl-fuzz -i seeds/ -o findings/ -- ./fuzz_hbw
```

### libfuzzer
Found out-of-memory leak due to never freeing the memory.
- `fuzz_hbw_libfuzzer.cpp` ran for 2 days and found a lot of memory leak
- `harness_store.cpp` this standalone fuzzer found memory leak: `oom_libfuzzer/oom-1a49c4b8af1f409501f75d8654909cd1d120ae8c` - check `memory_leak_new.png`
	- Error occurred here while [new workpiece](https://github.com/fischertechnik/txt_training_factory/blob/27526cc803ebfcecd1163de31f0e3c6d25f65ca8/TxtSmartFactoryLib/src/TxtHighBayWarehouseStorage.cpp#L208) created with `new` but never freed.
	- Updated the file to include destructor




