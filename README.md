
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

##### Print numbers as bits

```c++
int8_t constexpr ten {10};
std::println("Bits(10) = {}", Bits(ten));
BitsBase::stringFormat.format = Format::Hexadecimal;
std::println("Bits(10) = {}", Bits(ten));
```

```bash
Bits(10) = 1010
Bits(10) = 0xA
```
##### Convert bits (interpreted as two's complement) to numbers
```c++
std::string constexpr x7F {"0111 1111"}, x80 {"1000 0000"};
std::println(" 127  == Bits<int8_t>(x7F) : {}",  127 == Bits<int8_t>{x7F});
std::println("-128  == Bits<int8_t>(x80) : {}", -128 == Bits<int8_t>{x80});
std::println(" 128  == Bits<uint8_t>(x80): {}",  128U == Bits<uint8_t>{x80});
```
```bash
 127  == Bits<int8_t>(x7F) : true
-128  == Bits<int8_t>(x80) : true
 128  == Bits<uint8_t>(x80): true
```
##### Convert numbers to bits using two's complement
```c++
std::println("Bits<int8_t>{{-3}}   = {}", Bits<int8_t>{-0x3});
std::println("Bits<uint8_t>{{253}} = {}", Bits<uint8_t>{0xFD});
std::println("Bits<int16_t>{{-1}}  = {}", Bits<int16_t>{-0x1});
std::println("Bits<int8_t>{{1}}    = {}", Bits<int8_t>{0x1});
```
```bash
Bits<int8_t>(-3)   = 0xFD
Bits<uint8_t>(253) = 0xFD
Bits<int16_t>(-1)  = 0xFFFF
Bits<int16_t>(1)   = 0x1
```

Build with `-DBUILD_EXAMPLES=ON` to build [examples.cpp](./cpp/examples.cpp) 