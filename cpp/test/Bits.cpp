#include "gtest/gtest.h"

#include "Bits.h"

namespace bb = bits_and_bytes;

class Bits : public testing::Test {
public:
    void SetUp() override {
        bb::BitsBase::stringFormat = bb::DEFAULT_STRING_FORMAT;
        // This string makes the tests readable by splitting long strings into groups and including leading zeroes for
        // clarity
        bb::BitsBase::stringFormat.leadingZeroes = bb::LeadingZeroes::Include;
        bb::BitsBase::stringFormat.bitUnit = bb::BitUnit::Nibble;
    }

    static void disableLeadingZeroes() {
        bb::BitsBase::stringFormat.leadingZeroes = bb::LeadingZeroes::Suppress;
    }

    static void disableBitGrouping() {
        bb::BitsBase::stringFormat.bitUnit = bb::BitUnit::None;
    }

    static void enableHexaDecimalOutput() {
        bb::BitsBase::stringFormat.format = bb::Format::Hexadecimal;
    }

    static void groupByBytes() {
        bb::BitsBase::stringFormat.bitUnit = bb::BitUnit::Byte;
    }

    static void useSingleQuoteDelimiter() {
        bb::BitsBase::stringFormat.groupDelimiter = '\'';
    }

protected:
    inline static bb::StringFormat stringFormat { bb::DEFAULT_STRING_FORMAT };
};

TEST_F(Bits, WillSizeUnsignedIntegerTypesCorrectly) {
    disableLeadingZeroes();
    disableBitGrouping();

    ASSERT_EQ(1, bb::Bits{int8_t{0U}}.getString().length());
    ASSERT_EQ(16, bb::Bits{uint16_t{0xFFFF}}.getString().length());
    ASSERT_EQ(32, bb::Bits{uint32_t{0xFFFF'FFFF}}.getString().length());
    ASSERT_EQ(64, bb::Bits{uint64_t{0xFFFF'FFFF'FFFF'FFFF}}.getString().length());
}

TEST_F(Bits, WillGroupByNibbleWhenLeadingZeroesAreOff) {
    disableLeadingZeroes();
    ASSERT_EQ("0", bb::Bits{uint8_t{0}});
    ASSERT_EQ("10", bb::Bits{int16_t{2}});
    ASSERT_EQ("1111", bb::Bits{int32_t{15}});
    ASSERT_EQ("1 0000", bb::Bits{int32_t{16}});
    // With leading zeroes off, unsigned values should produce the same output for all
    // data types that can represent the number
    // NOTE: This invokes operator== with dissimilar types
    ASSERT_EQ(bb::Bits{int64_t{16}}, bb::Bits{int32_t{16}});
}

TEST_F(Bits, WillGroupByNibbleCorrectlyWhenLeadingZeroesAreOn) {
    ASSERT_EQ("0000 0000", bb::Bits{uint8_t{0}});
    ASSERT_EQ("0000 0000 0000 0010", bb::Bits{int16_t{2}});
    ASSERT_EQ("0000 0000 0000 0000 0000 0000 0000 1111", bb::Bits{int32_t{15}});
    ASSERT_EQ("0001 0000", bb::Bits{int8_t{16}});
}

TEST_F(Bits, WillProduceCorrectHexWhenLeadingZeroesAreOff) {
    enableHexaDecimalOutput();
    disableLeadingZeroes();
    ASSERT_EQ("0x 0", bb::Bits{uint8_t{0}});
    ASSERT_EQ("0x 2", bb::Bits{int16_t{2}});
    ASSERT_EQ("0x F", bb::Bits{int32_t{15}});
    ASSERT_EQ("0x 1 0", bb::Bits{int32_t{16}});
}

TEST_F(Bits, WillProduceCorrectHexWhenLeadingZeroesAreOn) {
    enableHexaDecimalOutput();
    ASSERT_EQ("0x 0 0", bb::Bits{uint8_t{0}});
    ASSERT_EQ("0x 0 0 0 2", bb::Bits{int16_t{2}});
    ASSERT_EQ("0x 0 0 0 0 0 0 0 F", bb::Bits{int32_t{15}});
    ASSERT_EQ("0x 0 0 0 0 0 0 1 0", bb::Bits{int32_t{16}});
}

TEST_F(Bits, WillProduceExpectedOutputWhenGroupedByBytes) {
    enableHexaDecimalOutput();
    groupByBytes();
    ASSERT_EQ("0x 00", bb::Bits{uint8_t{0}});
    ASSERT_EQ("0x 00 02", bb::Bits{int16_t{2}});
    ASSERT_EQ("0x 00 00 00 0F", bb::Bits{int32_t{15}});
    ASSERT_EQ("0x 00 00 00 10", bb::Bits{int32_t{16}});
}

TEST_F(Bits, WillUseUserProvidedDelimiterForGrouping) {
    groupByBytes();
    useSingleQuoteDelimiter();
    ASSERT_EQ("00000000", bb::Bits{uint8_t{0}});
    ASSERT_EQ("00000000'00000010", bb::Bits{int16_t{2}});
    ASSERT_EQ("00000000'00000000'00000000'00001111", bb::Bits{int32_t{15}});
    ASSERT_EQ("00010000", bb::Bits{int8_t{16}});
}

TEST_F(Bits, WillBuildFromHexaDecimalString) {
    ASSERT_EQ(0xAF, bb::Bits<int16_t>{"0xAF"}.getValue());
    ASSERT_EQ(0, bb::Bits<int16_t>{"0x0"}.getValue());
    ASSERT_THROW(
        try {
            bb::Bits<int8_t>{"0x1FF"};
        } catch (std::runtime_error const& ex) {
            ASSERT_STREQ(ex.what(), "Hexadecimal value 0x1FF (Decimal = 511) exceeds type's maximum 127");
            throw;
        }, std::runtime_error
    );
    ASSERT_EQ("1111 1111", bb::Bits<uint8_t>{"0xFF"});
    ASSERT_THROW(
        try {
            bb::Bits<int8_t>{"0x"};
        } catch (std::runtime_error const& ex) {
            ASSERT_STREQ(ex.what(), "0x is not a valid hexadecimal value.");
            throw;
        }, std::runtime_error
    );
}

TEST_F(Bits, WillBuildPositiveNumbersFromBinaryString) {
    ASSERT_EQ(0xAF, bb::Bits<uint16_t>{"1010 1111"}.getValue());
    ASSERT_EQ(0, bb::Bits<uint8_t>{"0 0"}.getValue());
    ASSERT_THROW(
        try {
            bb::Bits<int8_t>{"0 1111 1111"};
        } catch (std::runtime_error const& ex) {
            ASSERT_STREQ(ex.what(), "Binary value 011111111 (Decimal = 255) exceeds type's maximum 127");
            throw;
        },
    std::runtime_error);
    enableHexaDecimalOutput();
    ASSERT_EQ("0x F F", bb::Bits<uint8_t>{"1111 1111"});
}

TEST_F(Bits, WillBuildNegativeNumbersFromTwosComplementBinaryString) {
    ASSERT_EQ(-8, bb::Bits<int8_t>{"1111 1000"}.getValue());
    ASSERT_EQ(-10, bb::Bits<int8_t>{"11110110"}.getValue());
    ASSERT_EQ(-16, bb::Bits<int8_t>{"1111 0000"}.getValue());
    ASSERT_EQ(std::numeric_limits<int16_t>::min(), bb::Bits<int16_t>{"1000 0000 0000 0000"}.getValue());
}

TEST_F(Bits, WillBuildNegativeNumbersFromTwosComplementHexString) {
    ASSERT_EQ(-8, bb::Bits<int8_t>{"0xF8"}.getValue());
    ASSERT_EQ(-10, bb::Bits<int8_t>{"0xF6"}.getValue());
    ASSERT_EQ(-16, bb::Bits<int8_t>{"0xF0"}.getValue());
    ASSERT_EQ(std::numeric_limits<int16_t>::min(), bb::Bits<int16_t>{"0x8000"}.getValue());
}