
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <core/memory.h>
#include <core/diagnostic.h>

namespace core
{
    namespace tests
    {
        namespace
        {
            void TestAlloc(size_t i_size, size_t i_alignment, size_t i_alignment_offset)
            {
                void* mem = aligned_allocate(i_size, i_alignment, i_alignment_offset);

                assert(address_is_aligned(address_add(mem, i_alignment_offset), i_alignment));

                memset(mem, 33, i_size);

                aligned_deallocate(mem, i_size, i_alignment, i_alignment_offset);
            }

            void TestTryAlloc(size_t i_size, size_t i_alignment, size_t i_alignment_offset)
            {
                void* mem = try_aligned_allocate(i_size, i_alignment, i_alignment_offset);

                if (mem != nullptr)
                {
                    assert(address_is_aligned(address_add(mem, i_alignment_offset), i_alignment));

                    memset(mem, 33, i_size);

                    aligned_deallocate(mem, i_size, i_alignment, i_alignment_offset);
                }
            }
        }

        void Memory()
        {
            Print("Test: Core - Memory...");

            static_assert(is_power_of_2(1));
            static_assert(is_power_of_2(128));
            
            TestAlloc(4, 4, 0);
            TestAlloc(128, 128, 0);
            TestAlloc(128, 128, 8);

            TestTryAlloc(4, 4, 0);
            TestTryAlloc(128, 128, 0);
            TestTryAlloc(128, 128, 8);

            PrintLn("successful");
        }

    } // namespace tests

} // namespace core
