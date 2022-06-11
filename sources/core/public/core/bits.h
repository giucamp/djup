
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <assert.h>
#include <limits>

namespace djup
{
    /** Returns an unsigned integer in which only the bit with the speficied index is 1.
        The least significant bit has index 0. */
    template <typename UINT>
        constexpr UINT bit(unsigned i_index)
    {
        static_assert(std::numeric_limits<UINT>::is_integer && !std::numeric_limits<UINT>::is_signed);
        static_assert(std::numeric_limits<UINT>::radix == 2);
        assert(i_index < std::numeric_limits<UINT>::digits);
        return UINT(1) << i_index;
    }

    /** Returns an unsigned integer in which only the bits from i_index to i_index + i_count (exluded) are .
        The least significant bit has index 0. */
    template <typename UINT>
        constexpr UINT bits(unsigned i_index, unsigned i_count)
    {
        static_assert(std::numeric_limits<UINT>::is_integer && !std::numeric_limits<UINT>::is_signed);
        static_assert(std::numeric_limits<UINT>::radix == 2);
        assert(i_index + i_count <= std::numeric_limits<UINT>::digits);

        if(i_count == std::numeric_limits<UINT>::digits)
            return ~UINT(0);

        UINT base = (UINT(1) << i_count) - 1;
        return base << i_index;
    }

    /** Returns an unsigned integer in which only the bit with the speficied index is 1.
        The most  significant bit has index 0. */
    template <typename UINT>
        constexpr UINT bit_reverse(unsigned i_reverse_index)
    {
        static_assert(std::numeric_limits<UINT>::is_integer && !std::numeric_limits<UINT>::is_signed);
        static_assert(std::numeric_limits<UINT>::radix == 2);
        assert(i_reverse_index < std::numeric_limits<UINT>::digits);
        return UINT(1) << (std::numeric_limits<UINT>::digits - i_reverse_index - 1);
    }

    /** Returns an unsigned integer in which only the bits from i_index to i_index + i_count (exluded) are .
        The most significant bit has index 0. */
    template <typename UINT>
        constexpr UINT bits_reverse(unsigned i_reverse_index, unsigned i_count)
    {
        static_assert(std::numeric_limits<UINT>::is_integer && !std::numeric_limits<UINT>::is_signed);
        static_assert(std::numeric_limits<UINT>::radix == 2);
        assert(i_reverse_index + i_count <= std::numeric_limits<UINT>::digits);

        if(i_count == std::numeric_limits<UINT>::digits)
            return ~UINT(0);

        unsigned index = std::numeric_limits<UINT>::digits - i_reverse_index - i_count;
        UINT base = (UINT(1) << i_count) - 1;
        return base << index;
    }
}
