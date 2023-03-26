
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <core/to_chars.h>
#include <limits>
#include <private/has_fp_charconv.h>
#if HAS_FLOAT_CHARCONV
    #include <charconv>
#else
    #include <cstdio>
#endif

namespace core
{
    #if HAS_FLOAT_CHARCONV

        namespace
        {
            template <typename FLOAT> void FloatToChars(CharBufferView & i_dest, FLOAT i_number)
            {
                /* we use an intermediate buffer because otherwise there no way
                    to know the total size of the string if there is not enough
                    space in the buffer. */
                constexpr size_t buffer_size = 4 +
                    std::numeric_limits<FLOAT>::max_exponent10 +
                    std::numeric_limits<FLOAT>::max_digits10;
                char buffer[buffer_size];

                std::to_chars_result const result = std::to_chars(buffer, buffer + buffer_size, i_number);
                assert(result.ec == std::errc{}); // it shoudld never fail
                const size_t written_chars = result.ptr - buffer;
                i_dest << std::string_view(buffer, written_chars);
            }
        }

        void CharWriter<float>::operator() (CharBufferView & i_dest, float i_source) noexcept
        {
            FloatToChars(i_dest, i_source);
        }

        void CharWriter<double>::operator() (CharBufferView & i_dest, double i_source) noexcept
        {
            FloatToChars(i_dest, i_source);
        }

        void CharWriter<long double>::operator() (CharBufferView & i_dest, long double i_source) noexcept
        {
            FloatToChars(i_dest, i_source);
        }

    #else // #if HAS_FLOAT_CHARCONV

        void CharWriter<float>::operator() (CharBufferView & i_dest, float i_source) noexcept
        {
            constexpr size_t buffer_size = 1024;
            char buffer[buffer_size];

            snprintf(buffer, buffer_size - 1,
                "%.*g", std::numeric_limits<float>::max_digits10, i_source);
            i_dest << buffer;
        }

        void CharWriter<double>::operator() (CharBufferView & i_dest, double i_source) noexcept
        {
            constexpr size_t buffer_size = 1024;
            char buffer[buffer_size];

            snprintf(buffer, buffer_size - 1,
                "%.*g", std::numeric_limits<double>::max_digits10, i_source);
            i_dest << buffer;
        }

        void CharWriter<long double>::operator() (CharBufferView & i_dest, long double i_source) noexcept
        {
            constexpr size_t buffer_size = 1024;
            char buffer[buffer_size];

            snprintf(buffer, buffer_size - 1,
                "%.*Lg",
                std::numeric_limits<long double>::max_digits10, i_source);
            i_dest << buffer;
        }

    #endif // #if HAS_FLOAT_CHARCONV

} // namespace core
