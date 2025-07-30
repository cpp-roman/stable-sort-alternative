# stable-sort-alternative

stable-sort-alternative

## Build and Run

### With gcc

```sh
g++ main.cpp -o out && out
```

### With CMake

```sh
mkdir build && cd build
cmake ..
cmake --build .
main
```

### Cross-Compiling

```sh
cmake -B build-arm -DCMAKE_TOOLCHAIN_FILE=cmake/arm-linux-gnueabi.cmake
cmake --build build-arm

cmake -B build-riscv -DCMAKE_TOOLCHAIN_FILE=cmake/riscv64-linux-gnu.cmake
cmake --build build-riscv
```
