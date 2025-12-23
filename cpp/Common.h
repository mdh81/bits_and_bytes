#pragma once

#include <cstdint>

namespace bits_and_bytes {

    enum class Order : uint8_t {
       LittleEndian,
       BigEndian
    };

    enum class Format : uint8_t {
        Binary,
        HexaDecimal,
    };

    enum class HexFormat : uint8_t {
        UpperCase,
        LowerCase,
    };

    enum class BitUnit {
        Nibble,
        Byte,
        None
    };

    [[nodiscard]] inline uint8_t asValue(BitUnit const& bitUnit) {
        return bitUnit == BitUnit::Nibble ? 4 : 8;
    }

    enum class LeadingZeroes : uint8_t {
        Suppress,
        Include,
    };

    unsigned char constexpr NumBitsInOneByte {8U};
    unsigned char constexpr NumBitsInOneNibble {4U};
    char constexpr DEFAULT_GROUP_DELIMITER {' '};

    struct StringFormat {
        Order order;
        Format format;
        HexFormat hexFormat;
        BitUnit bitUnit;
        LeadingZeroes leadingZeroes;
        char groupDelimiter;
    };

    inline StringFormat constexpr DEFAULT_STRING_FORMAT {
        Order::BigEndian,
        Format::Binary,
        HexFormat::UpperCase,
        BitUnit::Nibble,
        LeadingZeroes::Include,
        DEFAULT_GROUP_DELIMITER
    };

}