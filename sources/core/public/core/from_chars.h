
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <core/to_chars.h>
#include <core/diagnostic.h>
#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string_view>
#include <type_traits>

namespace core
{
    /** Primary template for a parser. The second paramater can be used in partial
        sepcializations to simplify sfinae conditions. The function operator must
        function must be noexcept, and may be constexpr. */
    template <typename TYPE, typename SFINAE_CONDITION = VoidT<>> struct Parser
    {
        Expected<TYPE> operator () (std::string_view & i_source) noexcept = delete;
    };

    /* trait HasParser. Checks whether a Parser is defined for a type */
    template <typename TYPE, typename = VoidT<>> struct HasParser : std::false_type { };
    template <typename TYPE> struct HasParser<TYPE, VoidT<
            decltype(Parser<TYPE>{}(std::declval<std::string_view &>()))>>
        : std::true_type { };
    template <typename TYPE> constexpr bool HasParserV = HasParser<TYPE>::value;
    template <typename TYPE> using HasParserT = typename HasParser<TYPE>::type;

    // TryParse - can be used if the type has a Parser
    template <typename TYPE, typename = std::enable_if_t<HasParserV<TYPE>>>
        constexpr Expected<TYPE> TryParse(std::string_view & i_source) noexcept
    {
        static_assert(
            std::is_same_v<std::invoke_result_t<Parser<TYPE>, std::string_view &>, Expected<TYPE>>,
            "Parser<TYPE>{}(i_source) must return Expected<TYPE>");
        return Parser<TYPE>{}(i_source);
    }

    // TryParse from const std::string_view & - expects the source to be empty after parsing
    template <typename TYPE>
        constexpr Expected<TYPE> TryParse(const std::string_view & i_source) noexcept
    {
        std::string_view source(i_source);
        auto result = TryParse<TYPE>(source);
        if(!result)
            return result.GetError();
        if (!source.empty())
            return StaticCStrException("Unexpected tailing content");
        return result;
    }

    /* Parse - can be used if the type has a Parser - throws on error */
    template <typename TYPE, typename = std::enable_if_t<HasParserV<TYPE>>>
        constexpr TYPE Parse(std::string_view & i_source)
    {
        return *Parser<TYPE>{}(i_source);
    }

    /* Parse from const std::string_view & - defined if the type has a Parser
       Expects the source to be empty after parsing. Throws on error.  */
    template <typename TYPE, typename = std::enable_if_t<HasParserV<TYPE>>>
        constexpr TYPE Parse(const std::string_view & i_source)
    {
        return *TryParse<TYPE>(i_source);
    }

    /** TryAccept - tries to parse an exact value */
    template <typename TYPE, std::enable_if_t<HasParserV<TYPE>> * = nullptr>
        constexpr Expected<void> TryAccept(std::string_view & i_source, const TYPE & i_expected_value) noexcept
    {
        static_assert(HasCharWriterV<TYPE>, "The type must also have a CharWriter");

        auto source = i_source;
        Expected<TYPE> const found_value = TryParse<TYPE>(source);
        if (!found_value.HasValue())
        {
            // parsing failed, just propagate the error in found_value
            return found_value.GetError();
        }
        if (*found_value != i_expected_value)
        {
            return StaticCStrException("TryAccept - found value different from the expected");
        }
        else
        {
            i_source = source;
            return {};
        }
    }

    // TryAccept for strings - they don't have a parse
    constexpr Expected<void> TryAccept(std::string_view & i_source,
        const std::string_view & i_expected) noexcept
    {
        if(i_source.substr(0, i_expected.size()) == i_expected)
        {
            i_source.remove_prefix(i_expected.length());
            return {};
        }
        else
        {
            return StaticCStrException("Unexpected string");
        }
    }

    // TryAccept for chars
    constexpr Expected<void> TryAccept(std::string_view & i_source, char i_expected) noexcept
    {
        if (!i_source.empty() && i_source.front() == i_expected)
        {
            i_source.remove_prefix(1);
            return {};
        }
        else
        {
            return StaticCStrException("Unexpected char");
        }
    }

    /** TryAccept - parses an exact value */
    template <typename TYPE, typename RESULT = decltype(
            TryAccept(std::declval<std::string_view&>(), std::declval<const TYPE&>()))>
        constexpr void Accept(std::string_view & i_source, const TYPE & i_expected_value)
    {
        auto const result = TryAccept(i_source, i_expected_value);
        if(!result)
            throw result.GetError();
    }

    // accept spaces
    constexpr bool TryAcceptSpaces(std::string_view & i_source) noexcept
    {
        bool some_chars_accepted = false;
        while (!i_source.empty() && i_source.back() == ' ')
        {
            i_source.remove_prefix(1);
            some_chars_accepted = true;
        }
        return some_chars_accepted;
    }

    // Parser for signed integer
    template <typename INT_TYPE>
        struct Parser<INT_TYPE, std::enable_if_t<std::is_integral_v<INT_TYPE> && std::is_signed_v<INT_TYPE>>>
    {
        constexpr Expected<INT_TYPE> operator()(std::string_view & i_source) noexcept
        {
            const char * curr_digit = i_source.data();
            const char * const end_of_buffer = curr_digit + i_source.size();
            INT_TYPE result = 0;

            if (*curr_digit == '-')
            {
                curr_digit++;
                while (curr_digit < end_of_buffer)
                {
                    if (*curr_digit >= '0' && *curr_digit <= '9')
                    {
                        INT_TYPE const digit = *curr_digit - '0';
                        INT_TYPE const thereshold = (std::numeric_limits<INT_TYPE>::min() + digit) / 10;
                        if (result < thereshold)
                        {
                            return StaticCStrException("Overflow parsing integer");
                        }
                        result *= 10;
                        result -= digit;
                    }
                    else
                    {
                        break;
                    }
                    curr_digit++;
                }
            }
            else
            {
                while (curr_digit < end_of_buffer)
                {
                    if (*curr_digit >= '0' && *curr_digit <= '9')
                    {
                        INT_TYPE const digit = *curr_digit - '0';
                        INT_TYPE const thereshold = (std::numeric_limits<INT_TYPE>::max() - digit) / 10;
                        if (result > thereshold)
                        {
                            return StaticCStrException("Overflow parsing integer");
                        }
                        result *= 10;
                        result += digit;
                    }
                    else
                    {
                        break;
                    }
                    curr_digit++;
                }
            }

            const size_t accepted_digits = curr_digit - i_source.data();
            if (accepted_digits == 0)
            {
                return StaticCStrException("Expected integer");
            }
            else
            {
                i_source.remove_prefix(accepted_digits);
                return result;
            }
        }
    };

    // Parser for unsigned integer
    template <typename UINT_TYPE>
        struct Parser<UINT_TYPE, std::enable_if_t<std::is_integral_v<UINT_TYPE> && !std::is_signed_v<UINT_TYPE>>>
    {
        constexpr Expected<UINT_TYPE> operator()(std::string_view & i_source) noexcept
        {
            const char * curr_digit = i_source.data();
            const char * const end_of_buffer = curr_digit + i_source.size();
            UINT_TYPE result = 0;

            while (curr_digit < end_of_buffer)
            {
                if (*curr_digit >= '0' && *curr_digit <= '9')
                {
                    UINT_TYPE const digit = *curr_digit - '0';
                    UINT_TYPE const thereshold = (std::numeric_limits<UINT_TYPE>::max() - digit) / 10;
                    if (result > thereshold)
                    {
                        return StaticCStrException("Overflow parsing integer");
                    }
                    result *= 10;
                    result += digit;
                }
                else
                {
                    break;
                }
                curr_digit++;
            }

            size_t const accepted_digits = curr_digit - i_source.data();
            if (accepted_digits == 0)
            {
                return StaticCStrException("Expected integer");
            }
            else
            {
                i_source.remove_prefix(accepted_digits);
                return result;
            }
        }
    };

    template <> struct Parser<bool>
    {
        constexpr Expected<bool> operator()(std::string_view & i_source) noexcept
        {
            if (TryAccept(i_source, "true"))
                return true;

            if (TryAccept(i_source, "false"))
                return false;

            return StaticCStrException("Expected true or false");
        }
    };

    template <> struct Parser<float>
    {
        Expected<float> operator()(std::string_view & i_source) noexcept;
    };

    template <> struct Parser<double>
    {
        Expected<double> operator()(std::string_view & i_source) noexcept;
    };

    template <> struct Parser<long double>
    {
        Expected<long double> operator()(std::string_view & i_source) noexcept;
    };

} // namespace core
