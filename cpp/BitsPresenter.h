#pragma once

#include <cstdint>
#include "Common.h"

namespace bits_and_bytes {
    template<typename NumericType>
    class Bits;

    /// Helper class used by Bits<T> for printing its bits according to the chosen string format
    class BitsPresenter {
    public:
        BitsPresenter(StringFormat const& stringFormat, uint8_t const numBitsInFormattedOutput)
            : numBitsInFormattedOutput(numBitsInFormattedOutput)
            , stringFormat(stringFormat) {}

        template<typename NumericType>
        void format(Bits<NumericType> const& bits) const {
            formattedOutput = stringFormat.format == Format::Binary
                ? formatBinary(bits.asBits())
                : formatHex(bits.asHex());
        }

        [[nodiscard]]
        std::string const& getOutput() const {
            return formattedOutput;
        }

    private:
        [[nodiscard]]
        std::string formatBinary(std::string&& binaryString) const {
            if (stringFormat.leadingZeroes == LeadingZeroes::Include) {
                binaryString.resize(numBitsInFormattedOutput, '0');
            }
            reverseString(binaryString);
            if (auto [groupingEnabled, groupSize] = getGroupSize(false); groupingEnabled) {
                return groupBits(binaryString, groupSize);
            }
            return binaryString;
        }

        [[nodiscard]]
        std::string formatHex(std::string&& hexString) const {
            std::string result(
                stringFormat.leadingZeroes == LeadingZeroes::Include
                    ? numBitsInFormattedOutput / NUM_BITS_IN_ONE_NIBBLE
                    : hexString.size(), '0'
            );
            std::ranges::transform(hexString, result.begin(), [this](char const c) {
                if (stringFormat.hexFormat == HexFormat::LowerCase && std::isupper(c)) {
                    return static_cast<char>('a' + c - 'A');
                }
                if (stringFormat.hexFormat == HexFormat::UpperCase && std::islower(c)) {
                    return static_cast<char>('A' + c - 'a');
                }
                return c;
            });
            reverseString(result);
            if (auto [groupingEnabled, groupSize] = getGroupSize(true); groupingEnabled) {
                result = groupBits(result, groupSize);
                result = " " + result;
            }
            return "0x" + result;
        }

        [[nodiscard]]
        std::pair<bool, uint8_t> getGroupSize(bool const isHex) const {
            if (stringFormat.bitUnit == BitUnit::None) {
                return { false, 0 };
            }
            uint8_t groupSize =
                stringFormat.bitUnit == BitUnit::Byte
                ? NUM_BITS_IN_ONE_BYTE
                : NUM_BITS_IN_ONE_NIBBLE;
            groupSize =
                isHex
                ? groupSize / NUM_BITS_IN_ONE_NIBBLE
                : groupSize;
            return { true, groupSize };
        }

        [[nodiscard]]
        std::string groupBits(std::string_view const numStr, uint8_t const groupSize) const {
            if (numStr.size() <= groupSize) {
                return std::string{numStr};
            }
            std::string result;
            auto numGroups = numStr.size() / groupSize;
            if (numStr.size() % groupSize) {
                numGroups += 1;
            }
            auto const numSpaces= numGroups - 1U;
            auto const size = numStr.size() + numSpaces;
            result.resize(size);
            uint8_t digitCounter {};
            auto outItr = result.rbegin();
            for (auto itr = numStr.rbegin(); itr != numStr.rend(); ++outItr, ++itr) {
                if (digitCounter == groupSize) {
                    digitCounter = 1;
                    *outItr = stringFormat.groupDelimiter;
                    ++outItr;
                    *outItr = *itr;
                } else {
                    ++digitCounter;
                    *outItr = *itr;
                }
            }
            return result;
        }

        static void reverseString(std::string& str) {
            for (uint8_t i = static_cast<uint8_t>(str.size()) - 1U, j = 0U; i > j; --i, ++j) {
                std::swap(str[i], str[j]);
            }
        }

        uint8_t numBitsInFormattedOutput;
        StringFormat stringFormat;
        mutable std::string formattedOutput;
    };
}