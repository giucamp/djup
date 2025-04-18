
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <core/bits.h>
#include <core/diagnostic.h>

namespace core
{
    namespace tests
    {
        void Bits()
        {
            Print("Test: Core - Bits...");

            static_assert(bit<uint32_t>(0) == 1);
            static_assert(bit<uint32_t>(1) == 2);
            static_assert(bit<uint32_t>(2) == 4);
            static_assert(bit<uint32_t>(3) == 8);
            static_assert(bit<uint32_t>(31) == 0x80000000);

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

            static_assert(bit_reverse<uint8_t>(0) == 128);
            static_assert(bit_reverse<uint8_t>(1) == 64);
            static_assert(bit_reverse<uint8_t>(2) == 32);
            static_assert(bit_reverse<uint8_t>(3) == 16);
            static_assert(bit_reverse<uint8_t>(7) == 1);
            static_assert(bit_reverse<uint32_t>(31) == 1);
            static_assert(bit_reverse<uint32_t>(0) == 0x80000000);

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

            PrintLn("successful");
        }

    } // namespace tests

} // namespace core
