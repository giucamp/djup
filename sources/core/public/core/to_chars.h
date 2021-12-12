
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <core/span.h>
#include <array>
#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace djup
{
   /** Allows to write chars to a fixed length buffer, truncating when the
        buffer is over. The char sequence is not automatically null-terminated. */
    class CharBufferView
    {
    public:

        /** Constructs a writer with no buffer. It may be used to compute the
            size required for a buffer. */
        constexpr CharBufferView() noexcept {}

        constexpr CharBufferView(Span<char> i_dest_buffer) noexcept
            : m_next_char(i_dest_buffer.data()),
              m_remaining_size(static_cast<ptrdiff_t>(i_dest_buffer.size()))
        {
        }

        constexpr CharBufferView(char * i_buffer, size_t i_buffer_length) noexcept
            : CharBufferView(Span(i_buffer, i_buffer_length))
        {
        }

        /** Returns the number of charcters remaining in the buffer, which is negative
            in case of buffer overlow. */
        constexpr ptrdiff_t RemainingSize() const noexcept { return m_remaining_size; }

        void SkipParsedChars(size_t i_number_of_parsed_chars) noexcept
        {
            assert(i_number_of_parsed_chars <= size());
            m_next_char += i_number_of_parsed_chars;
            m_remaining_size -= i_number_of_parsed_chars;
        }

        constexpr void Append(char i_char) noexcept
        {
            if (--m_remaining_size >= 0)
                *m_next_char++ = i_char;
        }

        constexpr void Append(const std::string_view & i_string) noexcept
        {
            auto const length_to_write =
                std::min(m_remaining_size, static_cast<ptrdiff_t>(i_string.length()));

            m_remaining_size -= static_cast<ptrdiff_t>(i_string.length());

            if (length_to_write > 0)
            {
                for (ptrdiff_t index = 0; index < length_to_write; index++)
                    m_next_char[index] = i_string[index];
                m_next_char += length_to_write;
            }
        }

        constexpr char * data() const noexcept { return m_next_char; }

        constexpr size_t size() const noexcept { return m_remaining_size > 0 ? static_cast<size_t>(m_remaining_size) : 0; }

        constexpr bool IsTruncated() const noexcept { return m_remaining_size < 0; }

        size_t GetExtraRequiredSize() const
        {
            if(IsTruncated())
                return static_cast<size_t>(-m_remaining_size);
            else
                return 0;
        }

    private:
        char * m_next_char = nullptr;
        ptrdiff_t m_remaining_size = 0;
    };

    /** Primary template for a CharWriter. The second paramater can be used in partial
        sepcializations to simplify sfinae conditions. The function operator must be
        noexcept, and may be constexpr. */
    template <typename TYPE, typename SFINAE_CONDITION = void>
        struct CharWriter
    {
        void operator() (CharBufferView & i_dest, const TYPE & i_source) noexcept =  delete;
    };

    // trait HasCharWriter - detects whether a CharWriter is defined for a type
    template <typename, typename = void> struct HasCharWriter : std::false_type { };
    template <typename TYPE> struct HasCharWriter<TYPE, VoidT<decltype(
            CharWriter<TYPE>{}(std::declval<CharBufferView &>(), std::declval<const TYPE &>()))
        >> : std::true_type { };
    template <typename TYPE> using HasCharWriterT = typename HasCharWriter<TYPE>::type;
    template <typename TYPE> constexpr bool HasCharWriterV = HasCharWriter<TYPE>::value;

    // CharBufferView << TYPE for types with CharWriter
    template <typename TYPE, typename = std::enable_if_t<
        HasCharWriterV<TYPE>
        >> constexpr CharBufferView & operator << (CharBufferView & i_dest, const TYPE & i_source) noexcept
    {
        CharWriter<TYPE>{}(i_dest, i_source);
        return i_dest;
    }

    // CharWriter for strings and chars
    template <typename TYPE> struct CharWriter<TYPE,
        std::enable_if_t<std::is_constructible_v<std::string_view, TYPE> >
        >
    {
        constexpr void operator() (CharBufferView & i_dest, const TYPE & i_source) noexcept
        {
            i_dest.Append(std::string_view(i_source));
        }
    };
    template <> struct CharWriter<char>
    {
        constexpr void operator() (CharBufferView & i_dest, char i_source) noexcept
        {
            i_dest.Append(i_source);
        }
    };

    // CharWriter for pointers
    template <typename TYPE> struct CharWriter<TYPE, VoidT<
            std::enable_if_t< std::is_pointer_v<TYPE>
            && !std::is_constructible_v<std::string_view, TYPE>
        > >>
    {
        constexpr void operator() (CharBufferView & i_dest, TYPE i_source) noexcept
        {
            if(i_source == nullptr)
                i_dest << "null";
            else
                i_dest << "ptr to " << *i_source;
        }
    };
    template <> struct CharWriter<std::nullptr_t>
    {
        constexpr void operator() (CharBufferView & i_dest, std::nullptr_t) noexcept
        {
            i_dest << "null";
        }
    };

    // CharWriter for unsigned integers
    template <typename UINT_TYPE> struct CharWriter<UINT_TYPE, VoidT< std::enable_if_t<
        std::is_integral_v<UINT_TYPE> && !std::is_signed_v<UINT_TYPE> &&
        !std::is_same_v<UINT_TYPE, bool>> > >
    {
        constexpr void operator() (CharBufferView & i_dest, UINT_TYPE i_source) noexcept
        {
            constexpr UINT_TYPE ten = 10;
            constexpr int buffer_size = std::numeric_limits<UINT_TYPE>::digits10 + 1;
            char buffer[buffer_size] = {};
            size_t length = 0;
            do
            {
                buffer[length] = static_cast<char>('0' + i_source % ten);
                i_source /= ten;

                assert(length < buffer_size); // buffer too small?
                length++;

            } while (i_source > 0);

            // until C++20 std::reverse is not constexpr
            for (size_t index = 0; index < length / 2; index++)
            {
                auto const other_index = (length - 1) - index;
                auto tmp = buffer[index];
                buffer[index] = buffer[other_index];
                buffer[other_index] = tmp;
            }

            i_dest << std::string_view(buffer, length);
        }
    };

    // CharWriter for signed integers
    template <typename SINT_TYPE> struct CharWriter<SINT_TYPE, VoidT<std::enable_if_t<
        std::is_integral_v<SINT_TYPE> && std::is_signed_v<SINT_TYPE> &&
        !std::is_same_v<SINT_TYPE, bool>> > >
    {
        constexpr void operator() (CharBufferView & i_dest, SINT_TYPE i_source) noexcept
        {
            const bool is_negative = i_source < 0;

            constexpr SINT_TYPE ten = 10;
            constexpr int buffer_size = std::numeric_limits<SINT_TYPE>::digits10 + 1;
            char buffer[buffer_size] = {};
            size_t length = 0;
            /* note: if the number is negative, we can't just negate the sign and use the same algorithm,
               because the unary minus operator is lossy: for example, negating -128 as int8 produces an
               overflow, as 128 can't be represented as int8 */
            if (is_negative)
            {
                do
                {
                    /* note: we do not use the modulo operator %, because it has implementation-defined
                       behavior with non-positive operands. */
                    SINT_TYPE const new_value = i_source / ten;
                    buffer[length] = static_cast<char>('0' + new_value * ten - i_source);
                    i_source = new_value;
                    length++;

                    assert(length < buffer_size || i_source == 0); // buffer too small?
                } while (i_source != 0);
            }
            else
            {
                do
                {

                    buffer[length] = static_cast<char>('0' + i_source % ten);
                    length++;
                    i_source /= ten;

                    assert(length < buffer_size || i_source == 0); // buffer too small?
                } while (i_source != 0);
            }

            if (is_negative)
            {
                i_dest << '-';
            }

            // until C++20 std::reverse is not constexpr
            for (size_t index = 0; index < length / 2; index++)
            {
                auto const other_index = (length - 1) - index;
                auto tmp = buffer[index];
                buffer[index] = buffer[other_index];
                buffer[other_index] = tmp;
            }

            i_dest << std::string_view(buffer, length);
        }
    };

    // CharWriter for bool
    template <> struct CharWriter<bool>
    {
        constexpr void operator() (CharBufferView & i_dest, bool i_source) noexcept
        {
            if(i_source)
                i_dest << "true";
            else
                i_dest << "false";
        }
    };

    // CharWriter for floating points
    template <> struct CharWriter<float>
        { void operator() (CharBufferView & i_dest, float i_source) noexcept; };
    template <> struct CharWriter<double>
        { void operator() (CharBufferView & i_dest, double i_source) noexcept; };
    template <> struct CharWriter<long double>
        { void operator() (CharBufferView & i_dest, long double i_source) noexcept; };

    // CharWriter for Span
    template <typename TYPE> struct CharWriter<Span<TYPE>>
    {
        constexpr void operator() (CharBufferView & i_dest, Span<TYPE> i_source) noexcept
        {
            for (size_t i = 0; i < i_source.size(); i++)
            {
                if(i != 0)
                    i_dest << ", ";
                i_dest << i_source[i];
            }
        }
    };

    /* namespace detail
    {
        template <typename TUPLE, size_t... INDICES>
        constexpr void print_tuple_like(
            CharBufferView & o_dest, const TUPLE & i_source, std::index_sequence<INDICES...>) noexcept
        {
            (((INDICES + 1 < sizeof...(INDICES)) ? (o_dest << std::get<INDICES>(i_source) << ", ")
                : (o_dest << std::get<INDICES>(i_source))),
                ...);
        }
    } // namespace detail

    template <typename TUPLE, typename = decltype(std::tuple_size<TUPLE>::value)>
        constexpr CharBufferView & operator<<(CharBufferView & o_dest, const TUPLE & i_tuple) noexcept
    {
        detail::print_tuple_like(
            o_dest, i_tuple, std::make_index_sequence<std::tuple_size<TUPLE>::value>{});
        return o_dest;
    } */

    /** Stringize multiple objects to a buffer, truncating when the buffer is over
        Returns the number of bytes required by the buffer to avoid truncation. */
    template <typename... TYPE>
        constexpr size_t ToChars(Span<char> i_dest, const TYPE &... i_objects) noexcept
    {
        CharBufferView writer(i_dest);
        (writer << ... << i_objects);
        return static_cast<size_t>(static_cast<ptrdiff_t>(i_dest.size()) - writer.RemainingSize());
    }

    /** Stringize multiple objects to a char array. If the buffer is not big enough
        the behaviour is undefined. */
    template <size_t SIZE, typename... TYPE>
        constexpr std::array<char, SIZE> ToCharArray(const TYPE &... i_objects)
    {
        std::array<char, SIZE> dest{};
        CharBufferView       writer(dest.data(), SIZE);
        (writer << ... << i_objects);
        assert(writer.RemainingSize() >= 0);
        return dest;
    }

    /** Compute the required buffeer size to stringize multiple objects. */
    template <typename... TYPE> constexpr size_t CharArraySize(const TYPE &... i_objects)
    {
        CharBufferView writer;
        (writer << ... << i_objects);
        return static_cast<size_t>(-writer.RemainingSize());
    }

} // namespace djup
