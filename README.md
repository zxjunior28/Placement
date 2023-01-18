# Multi-chip Partition and Placement

## Build with Makefile directly 
```console
$ make clean
$ make
$ ./Lab3 "data/case0.txt" "data/ans/output_case0.txt"
```

## Build with CMake by scripts
```console
$ source scripts/boost.sh "data/case0.txt" "data/ans/output_case0.txt"
```

## Build with CMake directly 
```console
$ rm -rf build/
$ cmake -S . -B build/ -DBUILD_EXAMPLES=ON
$ cmake --build build/ -j4
$ ./build/bin/Lab3 "data/case0.txt" "data/ans/output_case0.txt"
```
## Verifier
```console
$ ./verifier [INPUT] [OUTPUT] 
```

## Draw
```console
$python3 draw.py [INPUT] [OUTPUT] [PICTURE_NAME]
```