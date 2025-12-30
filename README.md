
## Bit Viewer

This library is for visualizing numbers as bits

[![Quality](https://github.com/mdh81/bit_viewer/actions/workflows/cmake-single-platform.yml/badge.svg)](https://github.com/mdh81/bit_viewer/actions/workflows/cmake-single-platform.yml)

### Build

#### Prerequisites
* cmake
* python

#### Build

```bash
$ cd <this repo dir>
$ mkdir build
$ cmake -S . -B build -DBUILD_EXAMPLES=ON 
$ cmake --build build --parallel
$ ctest --test-dir build/
```

### Usage

#### C++

When cmake variable `BUILD_EXAMPLES` is `ON`, [examples.cpp](./cpp/examples.cpp) is built and produces this
output that documents the usage of this library

```bash
$ ./build/cpp/examples

---------------------------------Bits Usage----------------------------------

Print numbers as bits

Bits(10) prints as 1010
Bits(10) prints as 0xA when BitsBase::stringFormat.format == Format::Hexadecimal 

Converts bits to numbers by interpreting them as 2s complement

Bits<int8_t>{"0x7F"} == 127: true
Bits<int8_t>{"0x80"} == -128: true
Bits<uint8_t>{"0x80"} == 128: true 

Converts negative numbers to 2s complement bit sequence

Bits<int8_t>{-3} prints as 0xFD
Bits<int8_t>{-1} prints as 0xFF
```
