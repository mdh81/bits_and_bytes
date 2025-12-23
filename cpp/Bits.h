// ReSharper disable CppDFAUnreachableFunctionCall
#pragma once

#include <format>
#include <ostream>
#include <ranges>
#include <string>
#include "Common.h"


namespace bits_and_bytes {

    // TODO:
    // 2. Python module
    // 3. Class named Bytes that accepts a T or sequence of T and serializes them to a byte array

    struct BitsBase {
        inline static StringFormat stringFormat {
            Order::BigEndian,
            Format::Binary,
            HexFormat::UpperCase,
            BitUnit::Nibble,
            LeadingZeroes::Include
        };
    };

    template<typename NumericType>
    class Bits;

    class BitsPresenter {
    public:
        BitsPresenter(StringFormat const& stringFormat, uint8_t const numBitsInFormattedOutput)
            : numBitsInFormattedOutput(numBitsInFormattedOutput)
            , stringFormat(stringFormat) {}

        template<typename NumericType>
        void format(Bits<NumericType> const& bits) const {
            formattedOutput = stringFormat.format == Format::Binary ?
                formatBinary(bits.asBits()) : formatHex(bits.asHex());
        }

        [[nodiscard]] std::string const& getOutput() const {
            return formattedOutput;
        }

    private:
        [[nodiscard]] std::string formatBinary(std::string&& binaryString) const {
            if (stringFormat.leadingZeroes == LeadingZeroes::Include) {
                binaryString.resize(numBitsInFormattedOutput, '0');
            }
            reverseString(binaryString);
            if (auto [groupingEnabled, groupSize] = getGroupSize(false); groupingEnabled) {
                return groupBits(binaryString, groupSize);
            }
            return binaryString;
        }

        [[nodiscard]] std::string formatHex(std::string&& hexString) const {
            std::string result;
            if (stringFormat.leadingZeroes == LeadingZeroes::Include) {
                result.resize(numBitsInFormattedOutput / NumBitsInOneNibble, '0');
            } else {
                result.resize(hexString.size());
            }
            std::ranges::transform(hexString, result.begin(), [this](char const c) {
                if (stringFormat.hexFormat == HexFormat::LowerCase && isupper(c)) {
                    return static_cast<char>('a' + c - 'A');
                }
                if (stringFormat.hexFormat == HexFormat::UpperCase && islower(c)) {
                    return static_cast<char>('A' + c - 'a');
                }
                return c;
            });
            reverseString(result);
            if (auto [groupingEnabled, groupSize] = getGroupSize(true); groupingEnabled) {
                result = groupBits(result, groupSize);
            }
            return "0x " + result;
        }

        [[nodiscard]] std::pair<bool, uint8_t> getGroupSize(bool const isHex) const {
            if (stringFormat.bitUnit == BitUnit::None) {
                return { false, 0 };
            }
            uint8_t groupSize = stringFormat.bitUnit == BitUnit::Byte ? NumBitsInOneByte : NumBitsInOneNibble;
            groupSize = isHex ? groupSize / 4 : groupSize;
            return { true, groupSize };
        }

        [[nodiscard]]
        std::string groupBits(std::string_view const& numStr, uint8_t const groupSize) const {
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

    template<typename NumericType>
    class Bits final : public BitsBase {
    static_assert(std::is_integral_v<NumericType>);
    public:
        explicit Bits(NumericType value)
            : value(value) {}

        explicit Bits(std::string const& bitStr) {
            if (bitStr.empty()) throw std::invalid_argument("Binary string is empty");
            value = asDecimal(bitStr);
        }

        explicit Bits(char const* bitChars)
            : Bits(std::string{bitChars}) {
        }

        template<typename AnotherNumericType>
        bool operator==(Bits<AnotherNumericType> const& another) const {
            return this->value == another.getValue();
        }

        // NOTE: The returned pointer is guaranteed to be valid for the lifetime of this Bits object
        operator char const*() const { // NOLINT: Implicit conversion is the API
            if (!presenter) {
                presenter = std::make_optional<BitsPresenter>(stringFormat, getNumberOfBits());
                presenter->format(*this);
            }
            return presenter->getOutput().c_str();
        }

        // NOTE: Only invoked for explicit conversion to prevent ambiguity between this and const char* version
        explicit operator std::string() const {
            char const* str = *this;
            return str;
        }

        NumericType getValue() const { return value; }

    private:
        [[nodiscard]] static constexpr uint8_t getNumberOfBits() {
            return sizeof(NumericType) * NumBitsInOneByte;
        }

        [[nodiscard]] static constexpr uint8_t getNumberOfNibbles() {
            return getNumberOfBits() / NumBitsInOneNibble;
        }

        [[nodiscard]] std::string asBits() const {
            std::string binaryString;
            auto const numBitsInType = getNumberOfBits();
            NumericType number {value};
            binaryString.reserve(numBitsInType);
            do {
                auto bit = number % 2;
                binaryString.push_back(bit == 0 ? '0' : '1');
                number /= 2;
            } while (number);
            return binaryString;
        }

        [[nodiscard]] std::string asHex() const {
            std::string hexString{};
            auto number = value;
            do {
                auto hexDigit = number % 16;
                auto hexDigitChar { hexDigit <= 9 ? '0' : 'a'};
                hexString.push_back(hexDigit <= 9 ? hexDigitChar + hexDigit : hexDigitChar + (hexDigit - 10));
                number = number / 16;
            } while (number);
            return hexString;
        }

        [[nodiscard]] NumericType asDecimal(std::string const& binStr) {
            NumericType number{};
            uint8_t bitPos{};
            for (auto itr = binStr.crbegin(); itr != binStr.crend(); ++itr, ++bitPos) {
                number += *itr * (1 << bitPos);
            }
            return number;
        }

        NumericType value;
        mutable std::optional<BitsPresenter> presenter;
        friend class BitsPresenter;
    };

}

// Functor to support printing bits::Bits via std::format
template <typename NumericType>
struct std::formatter<bits_and_bytes::Bits<NumericType>> : std::formatter<std::string> {
    auto format(bits_and_bytes::Bits<NumericType> const& bits, std::format_context& ctx) const {
        return std::formatter<std::string>::format(static_cast<std::string>(bits), ctx);
    }
};

// Stream overload to print to output stream
template<typename NumericType>
std::ostream& operator << (std::ostream& os, bits_and_bytes::Bits<NumericType> const& bits) {
   os << static_cast<std::string>(bits) << std::endl;
   return os;
}