
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
        The least significant bit has index 0. 
        
        static_assert(bit<uint32_t>(0) == 1);
        static_assert(bit<uint32_t>(1) == 2);
        static_assert(bit<uint32_t>(2) == 4);
        static_assert(bit<uint32_t>(3) == 8);
        static_assert(bit<uint32_t>(31) == 0x80000000);
    */
    template <typename UINT>
        constexpr UINT bit(unsigned i_index)
    {
        static_assert(std::numeric_limits<UINT>::is_integer && !std::numeric_limits<UINT>::is_signed);
        static_assert(std::numeric_limits<UINT>::radix == 2);
        assert(i_index < std::numeric_limits<UINT>::digits);
        return UINT(1) << i_index;
    }

    /** Returns an unsigned integer in which only the bits from i_index to i_index + i_count (exluded) are .
        The least significant bit has index 0.
     
        static_assert(bits<uint32_t>(0, 1) == 1);
        static_assert(bits<uint32_t>(1, 1) == 2);
        static_assert(bits<uint32_t>(31, 1) == 0x80000000);
        static_assert(bits<uint32_t>(0, 0) == 0);
        static_assert(bits<uint32_t>(2, 0) == 0);
        static_assert(bits<uint32_t>(0, 3) == 7);
        static_assert(bits<uint32_t>(2, 3) == 7 << 2);
        static_assert(bits<uint32_t>(0, 32) == 0xFFFFFFFF);
        static_assert(bits<uint32_t>(1, 31) == 0xFFFFFFFF - 1);
        static_assert(bits<uint32_t>(31, 0) == 0);
    */
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
        The most  significant bit has index 0. 

        static_assert(bit_reverse<uint8_t>(0) == 128);
        static_assert(bit_reverse<uint8_t>(1) == 64);
        static_assert(bit_reverse<uint8_t>(2) == 32);
        static_assert(bit_reverse<uint8_t>(3) == 16);
        static_assert(bit_reverse<uint8_t>(7) == 1);
        static_assert(bit_reverse<uint32_t>(31) == 1);
        static_assert(bit_reverse<uint32_t>(0) == 0x80000000);    
    */
    template <typename UINT>
        constexpr UINT bit_reverse(unsigned i_reverse_index)
    {
        static_assert(std::numeric_limits<UINT>::is_integer && !std::numeric_limits<UINT>::is_signed);
        static_assert(std::numeric_limits<UINT>::radix == 2);
        assert(i_reverse_index < std::numeric_limits<UINT>::digits);
        return UINT(1) << (std::numeric_limits<UINT>::digits - i_reverse_index - 1);
    }

    /** Returns an unsigned integer in which only the bits from i_index to i_index + i_count (exluded) are .
        The most significant bit has index 0. 

        static_assert(bits_reverse<uint8_t>(0, 1) == 128);
        static_assert(bits_reverse<uint8_t>(0, 2) == 128 + 64);
        static_assert(bits_reverse<uint8_t>(6, 2) == 3);
        static_assert(bits_reverse<uint8_t>(0, 0) == 0);
        static_assert(bits_reverse<uint8_t>(7, 0) == 0);
        static_assert(bits_reverse<uint8_t>(1, 1) == 64);
        static_assert(bits_reverse<uint8_t>(2, 1) == 32);
        static_assert(bits_reverse<uint32_t>(31, 1) == 1);
        static_assert(bits_reverse<uint32_t>(29, 3) == 7);
        static_assert(bits_reverse<uint32_t>(0, 2) == 0xC0000000);
        static_assert(bits_reverse<uint32_t>(0, 32) == 0xFFFFFFFF);    
    */
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
