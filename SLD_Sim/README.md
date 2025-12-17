#### Create SLD simulation
```
g++ -std=c++11 -pthread main.cpp TxtSortingLine.cpp TxtSortingLineRun.cpp TxtSortingLineCalibData.cpp -o sld_sim
./sld_sim
```

#### Fuzz the SLD programs
```
g++ -std=c++11 -pthread fuzz_sld.cpp TxtSortingLine.cpp TxtSortingLineRun.cpp TxtSortingLineCalibData.cpp -o fuzz_sld
./fuzz_sld
```
