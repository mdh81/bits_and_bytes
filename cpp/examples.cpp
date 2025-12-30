#include "Bits.h"
#include <print>
#include <iostream>
#include <limits>

int main() {
    using namespace bits_and_bytes;

    std::println("\n{:-^80}", "Bits Usage");

    std::println("\nPrint numbers as bits\n");
    int8_t constexpr ten {10};
    std::println("Bits(10) = {}", Bits(ten));
    BitsBase::stringFormat.format = Format::Hexadecimal;
    std::println("Bits(10) = {}", Bits(ten));

    std::println("\nConvert bits to numbers by interpreting them as two's complement\n");
    std::println(" 127  == Bits<int8_t>(\"0111 1111\") : {}", Bits<int8_t>{"0111 1111"} == 0x7F);
    std::println("-128  == Bits<int8_t>(\"0x80\")      : {}", std::numeric_limits<int8_t>::min() == Bits<int8_t>{"0x80"});
    std::println(" 128  == Bits<uint8_t>(\"0x80\")     : {}", 128 == Bits<uint8_t>{"0x80"});

    std::println("\nConverts numbers to bits using two's complement\n");
    std::println("Bits<int8_t>{{-3}}   = {}", Bits<int8_t>{-0x3});
    std::println("Bits<uint8_t>{{253}} = {}", Bits<uint8_t>{0xFD});
    std::println("Bits<int16_t>{{-1}}  = {}", Bits<int16_t>{-0x1});
    std::println("Bits<int8_t>{{1}}    = {}", Bits<int8_t>{0x1});

    return EXIT_SUCCESS;
}
