#include "Bits.h"
#include <print>
#include <iostream>
#include <limits>

int main() {
    namespace bb = bits_and_bytes;

    std::println("\n{:-^80}", "Bits Usage");

    std::println("\nPrint numbers as bits\n");
    int8_t constexpr ten {10};
    std::println("Bits(10) prints as {}", bb::Bits(ten));
    bb::BitsBase::stringFormat.format = bb::Format::Hexadecimal;
    std::println("Bits(10) prints as {} when BitsBase::stringFormat.format == Format::Hexadecimal ", bb::Bits(ten));

    std::println("\nConverts bits to numbers by interpreting them as two's complement\n");
    int8_t value = bb::Bits<int8_t>{"0x7F"};
    std::println("Bits<int8_t>{{\"0x7F\"}}  == 127: {}", value == 0x7F);
    value = bb::Bits<int8_t>{"0x80"};
    std::println("Bits<int8_t>{{\"0x80\"}} == -128: {}", std::numeric_limits<int8_t>::min() == value);
    uint8_t const uval = bb::Bits<uint8_t>{"0x80"};
    std::println("Bits<uint8_t>{{\"0x80\"}} == 128: {}", abs(std::numeric_limits<int8_t>::min()) == uval);

    std::println("\nConverts negative numbers to two's complement bit sequence\n");
    std::println("Bits<int8_t>{{-3}} prints as {}", bb::Bits<int8_t>{-3});
    std::println("Bits<int8_t>{{-1}} prints as {}", bb::Bits<int8_t>{-1});

    return EXIT_SUCCESS;
}
