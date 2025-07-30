# BMP Reader

"BMP Reader" test task

## Building

This project uses CMake for building.
To build BMP Reader type:
```bash
cmake -S . -B build && cmake --build build
```

### CMake options

- `BUILD_TESTS` -- compile tests
- `BUILD_CLI` -- compile command-line interface

## Running

To run BMP Reader, type
```bash
build/target/BMPReader_cli
```
or
```bash
build/target/BMPReader_cli {input_filename} {x1} {y1} {x2} {y2} {output_filename}
```

## Running tests

To run tests tou can use CTest:
```bash
cd build
ctest
```
