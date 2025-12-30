// ReSharper disable CppDFAUnreachableFunctionCall
#pragma once

#include <format>
#include <ostream>
#include <ranges>
#include <string>
#include <algorithm>
#include <regex>
#include "Common.h"
#include "BitsPresenter.h"


namespace bits_and_bytes {

    /// Non-template base class that allows clients of Bits to set the string format globally for all template
    /// instantiations of Bits<T>
    struct BitsBase {
        inline static StringFormat stringFormat = DEFAULT_STRING_FORMAT;
    };

    template<typename NumericType>
    class Bits final : public BitsBase {
    static_assert(std::is_integral_v<NumericType>);
    public:
        /// @brief Constructs a bit sequence for the given number.
        ///
        /// If Bits<NumericType> is signed, the bit sequence will be in the two's complement form
        explicit Bits(NumericType value)
            : value(value) {}

        /// @brief Constructs a bit sequence from the bit string by zero extending and extracting a numerical value
        /// from it.
        ///
        /// If Bits<NumericType> is signed and if the MSB of the input bit string is 1, then bit string is assumed
        /// to be in two's complement form and a signed value is generated accordingly
        /// @exception OutOfRangeException bitString exceeds the bit width of this template type
        /// @exception BitFormatException bitString is not a valid hexadecimal or binary string
        explicit Bits(std::string_view const bitString)
            : value(convertToDecimal(bitString)) {
        }

        /// Compares this bits sequence to another bit sequence of potentially different bit width returning true
        /// if the underlying numeric values are equal
        template<typename AnotherNumericType>
        [[nodiscard]]
        bool operator==(Bits<AnotherNumericType> const& another) const {
            return this->value == another.getValue();
        }

        /// Implicitly converts Bits<NumericType> to NumericType
        [[nodiscard]]
        operator NumericType() const { // NOLINT: Implicit conversion is by design
            return value;
        }

        /// Compares this object to a formatted bit string by applying the current format to this object's bit sequence
        [[nodiscard]]
        bool operator==(std::string_view const bitString) const {
            return getString() == bitString;
        }

        /// Gets the numeric value of the bit representation
        [[nodiscard]]
        NumericType getValue() const {
            return value;
        }

        /// Gets the bit representation as a string using the current string format
        /// @see StringFormat
        [[nodiscard]]
        std::string_view getString() const {
            if (!presenter) {
                presenter = std::make_optional<BitsPresenter>(stringFormat, getNumberOfBits());
                presenter->format(*this);
            }
            return presenter->getOutput();
        }

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

        /// @brief Converts data member NumericType::value to a binary sequence of length sizeof(NumericType) * 8
        /// To make this conversion without relying on any implementation-defined behavior, this method simply converts a
        /// signed value to a variable of unsigned type of the same width. The language guarantees that conversion from
        /// a signed value to an unsigned value is well-defined, and it relies on modulo arithmetic and bit width of
        /// the unsigned type.
        ///
        /// Example:
        /// int8_t value = -3;             // 0xFD in two's complement form
        /// uint8_t unsignedValue = value; // Still 0xFD because conversion uses modulo 2^N arithmetic
        ///                                // where N = 8 = sizeof(uint8_t) * 8
        /// Therefore, unsignedValue = -3 % 256 = 253
        /// As a result, the following bit extraction loop becomes well-defined
        /// do {
        ///     auto bit = unsignedValue & 1U;
        ///     unsignedValue >>= 1;
        /// } while(unsignedValue);
        ///
        /// If the signed "value" was used in place of unsignedValue, this loop's behavior is implementation-defined.
        /// In clang 16 on macOS, this loop is infinite since right shifting -1 yields -1 (sign propagation shift)
        /// @note Left-shifting a negative value is UB already
        [[nodiscard]]
        std::string asBits() const {
            std::string binaryString;
            auto const numBitsInType = getNumberOfBits();
            using UnsignedNumericType = std::make_unsigned_t<NumericType>;
            UnsignedNumericType number = value;
            UnsignedNumericType constexpr ONE{1U};
            binaryString.reserve(numBitsInType);
            do {
                auto const bit = number & ONE;
                binaryString.push_back(bit == 0 ? '0' : '1');
                number >>= ONE;
            } while (number);
            return binaryString;
        }

        /// @brief Converts data member NumericType::value to a hexadecimal sequence of length (sizeof(NumericType) * 8)/4
        /// @see Bits<NumericType>::asBits()
        [[nodiscard]]
        std::string asHex() const {
            std::string hexString{};
            using UnsignedNumericType = std::make_unsigned_t<NumericType>;
            UnsignedNumericType number = value;
            do {
                auto hexDigit = number & 0xF;
                auto hexDigitChar { hexDigit <= NINE ? '0' : 'A' };
                hexString.push_back(hexDigit <= NINE ? hexDigitChar + hexDigit : hexDigitChar + (hexDigit - TEN));
                number >>= NUM_BITS_IN_ONE_NIBBLE;
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

        NumericType value;
        mutable std::optional<BitsPresenter> presenter;
        friend class BitsPresenter;
        inline static std::regex const HEX_REGEX {"0x[0-9A-Fa-f]{1,16}" };
        inline static std::regex const BIN_REGEX {"[0-1]{1,64}" };
        static constexpr NumericType MaxValue {std::numeric_limits<NumericType>::max()};
        static constexpr NumericType MinValue {std::numeric_limits<NumericType>::min()};
        bool inputIsHex{};
    };

    // Stream overload to print to output stream
    template<typename NumericType>
    std::ostream& operator << (std::ostream& os, Bits<NumericType> const& bits) {
        os << bits.getString() << std::endl;
        return os;
    }
}

// Custom formatter to support printing bits::Bits via std::format
template <typename NumericType>
struct std::formatter<bits_and_bytes::Bits<NumericType>> : std::formatter<std::string_view> {
    auto format(bits_and_bytes::Bits<NumericType> const& bits, std::format_context& ctx) const {
        return std::formatter<std::string_view>::format(bits.getString(), ctx);
    }
};
