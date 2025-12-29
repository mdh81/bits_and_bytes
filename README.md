
## Bit Viewer

This library is for visualizing numbers as bits

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

bb::Bits(10) prints as 1010
bb::Bits(10) prints as 0xA when BitsBase::stringFormat.format == Format::Hexadecimal 

Converts bits to numbers by interpreting them as twos complement

Bits<int8_t>{"0x7F"} == 127: true
Bits<int8_t>{"0x80"} == -128: true
Bits<uint8_t>{"0x80"} == 128: true 
```