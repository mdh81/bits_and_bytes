// ReSharper disable CppDFAUnreachableFunctionCall
#pragma once

#include <format>
#include <ostream>
#include <ranges>
#include <string>
#include <algorithm>
#include <regex>
#include "Common.h"


namespace bits_and_bytes {

    // TODO:
    // 2. Python module
    // 3. Class named Bytes that accepts a T or sequence of T and serializes them to a byte array

    /// Non-template base class that allows clients of Bits to set the string format globally for all template
    /// instantiations of Bits<T>
    struct BitsBase {
        inline static StringFormat stringFormat = DEFAULT_STRING_FORMAT;
    };

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

    template<typename NumericType>
    class Bits final : public BitsBase {
    static_assert(std::is_integral_v<NumericType>);
    public:
        explicit Bits(NumericType value)
            : value(value) {}

        /// Converts the given bit string to a zero extended binary string and extracts the signed decimal value
        /// @exception OutOfRangeException bitString exceeds the bit width of this template type
        /// @exception BitFormatException bitString is not a valid hexadecimal or binary string
        ///
        explicit Bits(std::string const& bitString)
            : value(convertToDecimal(bitString)) {
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

        /// Gets the numeric value of the bit representation
        [[nodiscard]]
        operator NumericType() const { // NOLINT: Implicit conversion is the api
            return value;
        }

        /// Gets the numeric value of the bit representation
        NumericType getValue() const { return value; }

    private:
        // NOTE: Private methods do not perform any sanity checks, it's expected that the public API checks the input
        // for validity before passing them to private methods for further processing

        NumericType convertToDecimal(std::string_view const bitString) {
            inputIsHex = bitString.starts_with("0x");
            return binaryAsDecimal(zeroExtend<NumericType>(bitString));
        }

        [[nodiscard]]
        static constexpr uint8_t getNumberOfBits() {
            return sizeof(NumericType) * NUM_BITS_IN_ONE_BYTE;
        }

        [[nodiscard]]
        static constexpr uint8_t getNumberOfNibbles() {
            return getNumberOfBits() / NUM_BITS_IN_ONE_NIBBLE;
        }

        [[nodiscard]]
        std::string asBits() const {
            std::string binaryString;
            auto const numBitsInType = getNumberOfBits();
            NumericType number {value};
            binaryString.reserve(numBitsInType);
            do {
                auto const bit = number & 1;
                binaryString.push_back(bit == 0 ? '0' : '1');
                number >>= 1;
            } while (number);
            return binaryString;
        }

        [[nodiscard]]
        std::string asHex() const {
            std::string hexString{};
            auto number = value;
            do {
                auto hexDigit = number % SIXTEEN;
                auto hexDigitChar { hexDigit <= NINE ? '0' : stringFormat.hexFormat == HexFormat::LowerCase ? 'a' : 'A' };
                hexString.push_back(hexDigit <= NINE ? hexDigitChar + hexDigit : hexDigitChar + (hexDigit - TEN));
                number = number / SIXTEEN;
            } while (number);
            return hexString;
        }

        [[nodiscard]]
        static NumericType interpretAsTwosComplement(std::string_view const binaryString) {
            int64_t rawValue{};
            int64_t placeValue{1};
            auto lsbBits = binaryString
                | std::views::reverse
                | std::views::take(binaryString.length() - 1);
            std::ranges::for_each(lsbBits, [&](auto const bit) {
                    rawValue += (bit - '0') * placeValue;
                    placeValue *= TWO;
                }
            );
            rawValue -= (binaryString[0] - '0') * placeValue;
            if (rawValue >= MinValue && rawValue <= MaxValue) {
                return static_cast<NumericType>(rawValue);
            }
            throw OutOfRangeException
            (
                std::format("Binary value {} (Decimal value = {}) outside the type's range "
                    "[{}, {}]", binaryString, rawValue, MinValue, MaxValue)
            );
        }

        [[nodiscard]]
        NumericType interpretAsUnsignedBinary(std::string_view const binaryString) {
            uint64_t rawValue{};
            uint8_t bitPos{};
            for (auto itr = binaryString.crbegin(); itr != binaryString.crend(); ++itr, ++bitPos) {
                rawValue += (*itr - '0') * (1 << bitPos);
            }
            if (rawValue <= MaxValue) {
                return static_cast<NumericType>(rawValue);
            }

            std::string errorPrefix = std::format("{} value {}",
                inputIsHex ? "Hexadecimal" : "Binary",
                inputIsHex ? convertBinaryToHexString(binaryString) : binaryString);
            throw OutOfRangeException(
                std::format("{} (Decimal = {}) exceeds type's maximum {}",
                    errorPrefix, rawValue, MaxValue)
            );
        }

        [[nodiscard]] NumericType binaryAsDecimal(std::string_view const binaryString) {
            return binaryString.starts_with('1') && std::is_signed_v<NumericType>
            ? interpretAsTwosComplement(binaryString)
            : interpretAsUnsignedBinary(binaryString);
        }

        [[nodiscard]] static uint8_t hexDigitAsDecimal(char const hexDigit) {
            if (hexDigit >= '0' && hexDigit <= '9') {
                return hexDigit - '0';
            }
            return std::tolower(hexDigit) - 'a' + TEN;
        }

        [[nodiscard]] NumericType interpretHexAsTwosComplement(std::string_view const hexStr) {
            // NOTE: Caller is expected to strip prefix 0x from the input argument
            std::string binStr;
            binStr.reserve(hexStr.size() * NUM_BITS_IN_ONE_NIBBLE);
            for (auto const hexDigit : hexStr) {
                for (auto const nibble = nibbleAsBits(hexDigitAsDecimal(hexDigit)); auto const bit : nibble) {
                    binStr.push_back(bit);
                }
            }
            return interpretAsTwosComplement(binStr);
        }

        [[nodiscard]] NumericType hexAsDecimal(std::string_view const hexStr) {
            if (hexDigitAsDecimal(hexStr[2]) >> 3) { // Check the sign bit
                return interpretHexAsTwosComplement(std::string_view{hexStr.cbegin() + 2, hexStr.cend() });
            }
            uint64_t rawValue{};
            uint64_t hexPlaceValue{1};
            for (char const digit : std::ranges::reverse_view(hexStr)) {
                if (digit == 'x') break;
                rawValue += hexDigitAsDecimal(digit) * hexPlaceValue;
                hexPlaceValue *= SIXTEEN;
            }
            if (rawValue <= MaxValue) {
                return static_cast<NumericType>(rawValue);
            }
            throw std::runtime_error
            (
                std::format("Hexa decimal value {} (Decimal = {}) exceeds type's maximum {}",
                    hexStr, rawValue, MaxValue)
            );
        }

        [[nodiscard]] static std::optional<std::string> getMatchingNumberString(std::string_view const str, std::regex const& expr) {
            std::string potentialNumStr;
            potentialNumStr.reserve(str.size());
            std::ranges::copy_if(str, std::back_inserter(potentialNumStr),
                [](char const c) { return !std::isspace(c); });
            std::smatch matches;
            std::regex_match(potentialNumStr, matches, expr);
            if (matches.ready() && matches.size() == 1) {
                return std::make_optional(potentialNumStr);
            }
            return std::nullopt;
        }

        [[nodiscard]] static std::optional<std::string> isValidHex(std::string_view const str) {
            return getMatchingNumberString(str, HEX_REGEX);

        }

        [[nodiscard]] static std::optional<std::string> isValidBinary(std::string_view const str) {
            return getMatchingNumberString(str, BIN_REGEX);
        }

        NumericType value;
        mutable std::optional<BitsPresenter> presenter;
        friend class BitsPresenter;
        inline static std::regex const HEX_REGEX {"0x[0-9A-Fa-f]{1,16}" };
        inline static std::regex const BIN_REGEX {"[0-1]{1,64}" };
        static constexpr NumericType MaxValue {std::numeric_limits<NumericType>::max()};
        static constexpr NumericType MinValue {std::numeric_limits<NumericType>::min()};
        bool inputIsHex{};
    };

}

// Custom formatter to support printing bits::Bits via std::format
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