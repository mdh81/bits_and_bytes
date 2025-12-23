#include "gtest/gtest.h"

#include "Bits.h"

class Bits : public testing::Test {
public:
    void SetUp() override {
        bits_and_bytes::BitsBase::stringFormat = bits_and_bytes::DEFAULT_STRING_FORMAT;
    }

    static void disableLeadingZeroes() {
        bits_and_bytes::BitsBase::stringFormat.leadingZeroes = bits_and_bytes::LeadingZeroes::Suppress;
    }

    static void disableBitGrouping() {
        bits_and_bytes::BitsBase::stringFormat.bitUnit = bits_and_bytes::BitUnit::None;
    }

    static void enableHexaDecimalOutput() {
        bits_and_bytes::BitsBase::stringFormat.format = bits_and_bytes::Format::HexaDecimal;
    }

    static void groupByBytes() {
        bits_and_bytes::BitsBase::stringFormat.bitUnit = bits_and_bytes::BitUnit::Byte;
    }

    static void useSingleQuoteDelimiter() {
        bits_and_bytes::BitsBase::stringFormat.groupDelimiter = '\'';
    }

protected:
    inline static bits_and_bytes::StringFormat stringFormat { bits_and_bytes::DEFAULT_STRING_FORMAT };
};

TEST_F(Bits, WillSizeUnsignedIntegerTypesCorrectly) {
    disableLeadingZeroes();
    disableBitGrouping();

    std::string bitStr{};

    bitStr = bits_and_bytes::Bits{uint8_t{0}};
    ASSERT_EQ(1, bitStr.length());

    bitStr = bits_and_bytes::Bits{uint16_t{0xFFFF}};
    ASSERT_EQ(16, bitStr.length());

    bitStr = bits_and_bytes::Bits{uint32_t{0xFFFF'FFFF}};
    ASSERT_EQ(32, bitStr.length());

    bitStr = bits_and_bytes::Bits{uint64_t{0xFFFF'FFFF'FFFF'FFFF}};
    ASSERT_EQ(64, bitStr.length());
}

TEST_F(Bits, WillGroupByNibbleWhenLeadingZeroesAreOff) {
    disableLeadingZeroes();
    ASSERT_STREQ("0", bits_and_bytes::Bits{uint8_t{0}});
    ASSERT_STREQ("10", bits_and_bytes::Bits{int16_t{2}});
    ASSERT_STREQ("1111", bits_and_bytes::Bits{int32_t{15}});
    ASSERT_STREQ("1 0000", bits_and_bytes::Bits{int32_t{16}});
    // With leading zeroes off, unsigned values should produce the same output for all
    // data types that can represent the number
    // NOTE: This invokes operator== with dissimilar types
    ASSERT_EQ(bits_and_bytes::Bits{int64_t{16}}, bits_and_bytes::Bits{int32_t{16}});
}

TEST_F(Bits, WillGroupByNibbleCorrectlyWhenLeadingZeroesAreOn) {
    ASSERT_STREQ("0000 0000", bits_and_bytes::Bits{uint8_t{0}});
    ASSERT_STREQ("0000 0000 0000 0010", bits_and_bytes::Bits{int16_t{2}});
    ASSERT_STREQ("0000 0000 0000 0000 0000 0000 0000 1111", bits_and_bytes::Bits{int32_t{15}});
    ASSERT_STREQ("0001 0000", bits_and_bytes::Bits{int8_t{16}});
}

TEST_F(Bits, WillProduceCorrectHexWhenLeadingZeroesAreOff) {
    enableHexaDecimalOutput();
    disableLeadingZeroes();
    ASSERT_STREQ("0x 0", bits_and_bytes::Bits{uint8_t{0}});
    ASSERT_STREQ("0x 2", bits_and_bytes::Bits{int16_t{2}});
    ASSERT_STREQ("0x F", bits_and_bytes::Bits{int32_t{15}});
    ASSERT_STREQ("0x 1 0", bits_and_bytes::Bits{int32_t{16}});
}

TEST_F(Bits, WillProduceCorrectHexWhenLeadingZeroesAreOn) {
    enableHexaDecimalOutput();
    ASSERT_STREQ("0x 0 0", bits_and_bytes::Bits{uint8_t{0}});
    ASSERT_STREQ("0x 0 0 0 2", bits_and_bytes::Bits{int16_t{2}});
    ASSERT_STREQ("0x 0 0 0 0 0 0 0 F", bits_and_bytes::Bits{int32_t{15}});
    ASSERT_STREQ("0x 0 0 0 0 0 0 1 0", bits_and_bytes::Bits{int32_t{16}});
}

TEST_F(Bits, WillProduceExpectedOutputWhenGroupedByBytes) {
    enableHexaDecimalOutput();
    groupByBytes();
    ASSERT_STREQ("0x 00", bits_and_bytes::Bits{uint8_t{0}});
    ASSERT_STREQ("0x 00 02", bits_and_bytes::Bits{int16_t{2}});
    ASSERT_STREQ("0x 00 00 00 0F", bits_and_bytes::Bits{int32_t{15}});
    ASSERT_STREQ("0x 00 00 00 10", bits_and_bytes::Bits{int32_t{16}});
}

TEST_F(Bits, WillUseUserProvidedDelimiterForGrouping) {
    groupByBytes();
    useSingleQuoteDelimiter();
    ASSERT_STREQ("00000000", bits_and_bytes::Bits{uint8_t{0}});
    ASSERT_STREQ("00000000'00000010", bits_and_bytes::Bits{int16_t{2}});
    ASSERT_STREQ("00000000'00000000'00000000'00001111", bits_and_bytes::Bits{int32_t{15}});
    ASSERT_STREQ("00010000", bits_and_bytes::Bits{int8_t{16}});
}