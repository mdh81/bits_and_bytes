#include "Bits.h"
#include <print>
#include <iostream>
#include <limits>

int main() {
    namespace bb = bits_and_bytes;

    std::println("\n{:-^80}", "Bits Usage");

    std::println("\nPrint numbers as bits\n");
    int8_t constexpr ten {10};
    std::println("Bits(10) = {}", bb::Bits(ten));
    bb::BitsBase::stringFormat.format = bb::Format::Hexadecimal;
    std::println("Bits(10) = {}", bb::Bits(ten));

    std::println("\nConverts bits to numbers by interpreting them as two's complement\n");
    std::println(" 127  == Bits<int8_t>(\"0111 1111\") : {}", bb::Bits<int8_t>{"0111 1111"} == 0x7F);
    std::println("-128  == Bits<int8_t>(\"0x80\")      : {}", std::numeric_limits<int8_t>::min() == bb::Bits<int8_t>{"0x80"});
    std::println(" 128  == Bits<uint8_t>(\"0x80\")    : {}", abs(std::numeric_limits<int8_t>::min()) == bb::Bits<uint8_t>{"0x80"});

    std::println("\nConverts numbers to bits using two's complement\n");
    std::println("Bits<int8_t>{{-3}}   = {}", bb::Bits<int8_t>{-0x3});
    std::println("Bits<uint8_t>{{253}} = {}", bb::Bits<uint8_t>{0xFD});
    std::println("Bits<int16_t>{{-1}}  = {}", bb::Bits<int16_t>{-0x1});
    std::println("Bits<int16_t>{{1}}   = {}", bb::Bits<int8_t>{0x1});

    return EXIT_SUCCESS;
}
