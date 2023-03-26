
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2016-2018.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// original source: https://github.com/giucamp/density/blob/master/include/density/density_common.h

#pragma once
#include <cstddef>
#include <limits>
#include <new>
#include <type_traits>
#include <utility>
#include <assert.h>
#include <core/algorithms.h>

namespace core
{
    /** Returns true whether the given unsigned integer number is a power of 2 (1, 2, 4, 8, ...)
        @param i_number must be > 0, otherwise the behavior is undefined */
    constexpr bool is_power_of_2(size_t i_number) noexcept
    {
        assert(i_number > 0);
        return (i_number & (i_number - 1)) == 0;
    }

    /** Returns true whether the given address has the specified alignment
        @param i_address address to be checked
        @param i_alignment must be > 0 and a power of 2 */
    inline bool address_is_aligned(const void * i_address, size_t i_alignment) noexcept
    {
        assert(is_power_of_2(i_alignment));
        assert(i_alignment > 0);
        return (reinterpret_cast<uintptr_t>(i_address) & (i_alignment - 1)) == 0;
    }

    /** Returns true whether the given unsigned integer has the specified alignment
        @param i_uint integer to be checked
        @param i_alignment must be > 0 and a power of 2 */
    template <typename UINT> inline bool uint_is_aligned(UINT i_uint, UINT i_alignment) noexcept
    {
        static_assert(
          std::numeric_limits<UINT>::is_integer && !std::numeric_limits<UINT>::is_signed,
          "UINT mus be an unsigned integer");
        assert(is_power_of_2(i_alignment));
        assert(i_alignment > 0);
        return (i_uint & (i_alignment - 1)) == 0;
    }

    /** Adds an offset to a pointer.
        @param i_address source address
        @param i_offset number to add to the address
        @return i_address plus i_offset */
    inline void * address_add(void * i_address, size_t i_offset) noexcept
    {
        const uintptr_t uint_pointer = reinterpret_cast<uintptr_t>(i_address);
        return reinterpret_cast<void *>(uint_pointer + i_offset);
    }

    /** Adds an offset to a pointer.
        @param i_address source address
        @param i_offset number to add to the address
        @return i_address plus i_offset */
    inline const void * address_add(const void * i_address, size_t i_offset) noexcept
    {
        const uintptr_t uint_pointer = reinterpret_cast<uintptr_t>(i_address);
        return reinterpret_cast<const void *>(uint_pointer + i_offset);
    }

    /** Subtracts an offset from a pointer
        @param i_address source address
        @param i_offset number to subtract from the address
        @return i_address minus i_offset */
    inline void * address_sub(void * i_address, size_t i_offset) noexcept
    {
        const uintptr_t uint_pointer = reinterpret_cast<uintptr_t>(i_address);
        assert(uint_pointer >= i_offset);
        return reinterpret_cast<void *>(uint_pointer - i_offset);
    }

    /** Subtracts an offset from a pointer
        @param i_address source address
        @param i_offset number to subtract from the address
        @return i_address minus i_offset */
    inline const void * address_sub(const void * i_address, size_t i_offset) noexcept
    {
        const uintptr_t uint_pointer = reinterpret_cast<uintptr_t>(i_address);
        assert(uint_pointer >= i_offset);
        return reinterpret_cast<const void *>(uint_pointer - i_offset);
    }

    /** Computes the unsigned difference between two pointers. The first must be above or equal to the second.
        @param i_end_address first address
        @param i_start_address second address
        @return i_end_address minus i_start_address */
    inline uintptr_t address_diff(const void * i_end_address, const void * i_start_address) noexcept
    {
        assert(i_end_address >= i_start_address);

        const uintptr_t end_uint_pointer   = reinterpret_cast<uintptr_t>(i_end_address);
        const uintptr_t start_uint_pointer = reinterpret_cast<uintptr_t>(i_start_address);

        return end_uint_pointer - start_uint_pointer;
    }

    /** Returns the biggest aligned address lesser than or equal to a given address
        @param i_address address to be aligned
        @param i_alignment alignment required from the pointer. It must be an integer power of 2.
        @return the aligned address */
    inline void * address_lower_align(void * i_address, size_t i_alignment) noexcept
    {
        assert(is_power_of_2(i_alignment));
        assert(i_alignment > 0);

        const uintptr_t uint_pointer = reinterpret_cast<uintptr_t>(i_address);

        const size_t mask = i_alignment - 1;

        return reinterpret_cast<void *>(uint_pointer & ~mask);
    }

    /** Returns the biggest aligned address lesser than or equal to a given address
        @param i_address address to be aligned
        @param i_alignment alignment required from the pointer. It must be an integer power of 2.
        @return the aligned address */
    inline const void * address_lower_align(const void * i_address, size_t i_alignment) noexcept
    {
        assert(is_power_of_2(i_alignment));
        assert(i_alignment > 0);

        const uintptr_t uint_pointer = reinterpret_cast<uintptr_t>(i_address);

        const size_t mask = i_alignment - 1;

        return reinterpret_cast<void *>(uint_pointer & ~mask);
    }

    /** Returns the biggest address lesser than the first parameter, such that i_address + i_alignment_offset is aligned
        @param i_address address to be aligned
        @param i_alignment alignment required from the pointer. It must be an integer power of 2
        @param i_alignment_offset alignment offset
        @return the result address */
    inline void *
      address_lower_align(void * i_address, size_t i_alignment, size_t i_alignment_offset) noexcept
    {
        void * address = address_add(i_address, i_alignment_offset);

        address = address_lower_align(address, i_alignment);

        address = address_sub(address, i_alignment_offset);

        return address;
    }

    /** Returns the biggest address lesser than the first parameter, such that i_address + i_alignment_offset is aligned
        @param i_address address to be aligned
        @param i_alignment alignment required from the pointer. It must be an integer power of 2
        @param i_alignment_offset alignment offset
        @return the result address */
    inline const void * address_lower_align(
      const void * i_address, size_t i_alignment, size_t i_alignment_offset) noexcept
    {
        const void * address = address_add(i_address, i_alignment_offset);

        address = address_lower_align(address, i_alignment);

        address = address_sub(address, i_alignment_offset);

        return address;
    }

    /** Returns the smallest aligned address greater than or equal to a given address
        @param i_address address to be aligned
        @param i_alignment alignment required from the pointer. It must be an integer power of 2.
        @return the aligned address */
    inline void * address_upper_align(void * i_address, size_t i_alignment) noexcept
    {
        assert(is_power_of_2(i_alignment));
        assert(i_alignment > 0);

        const uintptr_t uint_pointer = reinterpret_cast<uintptr_t>(i_address);

        const size_t mask = i_alignment - 1;

        return reinterpret_cast<void *>((uint_pointer + mask) & ~mask);
    }

    /** Returns the smallest aligned address greater than or equal to a given address
        @param i_address address to be aligned
        @param i_alignment alignment required from the pointer. It must be an integer power of 2.
        @return the aligned address */
    inline const void * address_upper_align(const void * i_address, size_t i_alignment) noexcept
    {
        assert(is_power_of_2(i_alignment));
        assert(i_alignment > 0);

        const uintptr_t uint_pointer = reinterpret_cast<uintptr_t>(i_address);

        const size_t mask = i_alignment - 1;

        return reinterpret_cast<void *>((uint_pointer + mask) & ~mask);
    }

    /** Returns the smallest aligned integer greater than or equal to a given address
        @param i_uint integer to be aligned
        @param i_alignment alignment required from the pointer. It must be an integer power of 2.
        @return the aligned address */
    template <typename UINT>
    constexpr UINT uint_upper_align(UINT i_uint, size_t i_alignment) noexcept
    {
        static_assert(
          std::numeric_limits<UINT>::is_integer && !std::numeric_limits<UINT>::is_signed,
          "UINT must be an unsigned integer");
        return (i_uint + (i_alignment - 1)) & ~(i_alignment - 1);
    }


    /** Returns the biggest aligned address lesser than or equal to a given address
        @param i_uint integer to be aligned
        @param i_alignment alignment required from the pointer. It must be an integer power of 2.
        @return the aligned address */
    template <typename UINT>
    constexpr UINT uint_lower_align(UINT i_uint, size_t i_alignment) noexcept
    {
        static_assert(
          std::numeric_limits<UINT>::is_integer && !std::numeric_limits<UINT>::is_signed,
          "UINT must be an unsigned integer");
        return i_uint & ~(i_alignment - 1);
    }

    /** Returns the smallest address greater than the first parameter, such that i_address + i_alignment_offset is aligned
        @param i_address address to be aligned
        @param i_alignment alignment required from the pointer. It must be an integer power of 2
        @param i_alignment_offset alignment offset
        @return the result address */
    inline void *
      address_upper_align(void * i_address, size_t i_alignment, size_t i_alignment_offset) noexcept
    {
        void * address = address_add(i_address, i_alignment_offset);

        address = address_upper_align(address, i_alignment);

        address = address_sub(address, i_alignment_offset);

        return address;
    }

    /** Returns the smallest address greater than the first parameter, such that i_address + i_alignment_offset is aligned
        @param i_address address to be aligned
        @param i_alignment alignment required from the pointer. It must be an integer power of 2
        @param i_alignment_offset alignment offset
        @return the result address */
    inline const void * address_upper_align(
      const void * i_address, size_t i_alignment, size_t i_alignment_offset) noexcept
    {
        const void * address = address_add(i_address, i_alignment_offset);

        address = address_upper_align(address, i_alignment);

        address = address_sub(address, i_alignment_offset);

        return address;
    }

    namespace detail
    {
        struct AlignmentHeader
        {
            void * m_block;
        };

        // old versions of libstdc++ define max_align_t only outside std::
#if defined(__GLIBCXX__)
        constexpr size_t MaxAlignment = alignof(max_align_t);
#else
        constexpr size_t MaxAlignment = alignof(std::max_align_t);
#endif

    } // namespace detail


    /** Uses the global operator new to allocate a memory block with at least the specified size and alignment
            @param i_size size of the requested memory block, in bytes
            @param i_alignment alignment of the requested memory block, in bytes
            @param i_alignment_offset offset of the block to be aligned, in bytes. The alignment is guaranteed only at i_alignment_offset
                from the beginning of the block.
            @return address of the new memory block

            \pre The behavior is undefined if either:
                - i_alignment is zero or it is not an integer power of 2
                - i_size is not a multiple of i_alignment
                - i_alignment_offset is greater than i_size

        A violation of any precondition results in undefined behavior.

        \n <b>Throws</b>: std::bad_alloc if the allocation fails
        \n <b>Progress guarantee</b>: the same of the built-in operator new (usually blocking)


        The content of the newly allocated block is undefined. */
    inline void * aligned_allocate(size_t i_size, size_t i_alignment, size_t i_alignment_offset = 0)
    {
        assert(is_power_of_2(i_alignment));
        assert(i_alignment > 0);
        assert(i_alignment_offset <= i_size);

        void * user_block;
        if (i_alignment <= detail::MaxAlignment && i_alignment_offset == 0)
        {
            user_block = operator new(i_size);
        }
        else
        {
            // reserve an additional space in the block equal to the max(i_alignment, sizeof(AlignmentHeader) - sizeof(void*) )
            size_t const extra_size =
                Max(i_alignment, sizeof(detail::AlignmentHeader));
            size_t const actual_size  = i_size + extra_size;
            auto const complete_block = operator new(actual_size);
            user_block                = address_lower_align(
              address_add(complete_block, extra_size), i_alignment, i_alignment_offset);
            detail::AlignmentHeader & header =
              *(static_cast<detail::AlignmentHeader *>(user_block) - 1);
            header.m_block = complete_block;
            assert(
              user_block >= complete_block &&
              address_add(user_block, i_size) <= address_add(complete_block, actual_size));
        }
        return user_block;
    }

    /** Uses the global operator new to try to allocate a memory block with at least the specified size and alignment.
        Returns nullptr in case of failure.
            @param i_size size of the requested memory block, in bytes
            @param i_alignment alignment of the requested memory block, in bytes
            @param i_alignment_offset offset of the block to be aligned, in bytes. The alignment is guaranteed only at i_alignment_offset
                from the beginning of the block.
            @return address of the new memory block, or nullptr in case of failure

            \pre The behavior is undefined if either:
                - i_alignment is zero or it is not an integer power of 2
                - i_size is not a multiple of i_alignment
                - i_alignment_offset is greater than i_size

        A violation of any precondition results in undefined behavior.

        \n <b>Throws</b>: nothing
        \n <b>Progress guarantee</b>: the same of the built-in operator new (usually blocking)


        The content of the newly allocated block is undefined. */
    inline void * try_aligned_allocate(
      size_t i_size, size_t i_alignment, size_t i_alignment_offset = 0) noexcept
    {
        assert(is_power_of_2(i_alignment));
        assert(i_alignment > 0);
        assert(i_alignment_offset <= i_size);

        void * user_block;
        if (i_alignment <= detail::MaxAlignment && i_alignment_offset == 0)
        {
            user_block = operator new(i_size, std::nothrow);
        }
        else
        {
            // reserve an additional space in the block equal to the max(i_alignment, sizeof(AlignmentHeader) - sizeof(void*) )
            size_t const extra_size =
              Max(i_alignment, sizeof(detail::AlignmentHeader));
            size_t const actual_size  = i_size + extra_size;
            auto const complete_block = operator new(actual_size, std::nothrow);
            if (complete_block != nullptr)
            {
                user_block = address_lower_align(
                  address_add(complete_block, extra_size), i_alignment, i_alignment_offset);
                detail::AlignmentHeader & header =
                  *(static_cast<detail::AlignmentHeader *>(user_block) - 1);
                header.m_block = complete_block;
                assert(
                  user_block >= complete_block &&
                  address_add(user_block, i_size) <= address_add(complete_block, actual_size));
            }
            else
            {
                user_block = nullptr;
            }
        }
        return user_block;
    }

    /** Deallocates a memory block allocated by aligned_allocate, using the global operator delete.
        After the call any access to the memory block results in undefined behavior.
            @param i_block block to deallocate, or nullptr.
            @param i_size size of the block to deallocate, in bytes.
            @param i_alignment alignment of the memory block.
            @param i_alignment_offset offset of the alignment

            \pre The behavior is undefined if either:
                - i_block is not a memory block allocated by the function allocate
                - i_size, i_alignment and i_alignment_offset are not the same specified when the block was allocated

        \n <b>Throws</b>: nothing
        \n <b>Progress guarantee</b>: the same of the built-in operator delete (usually blocking)


        If i_block is nullptr, the call has no effect. */
    inline void aligned_deallocate(
      void * i_block, size_t i_size, size_t i_alignment, size_t i_alignment_offset = 0) noexcept
    {
        assert(is_power_of_2(i_alignment));
        assert(i_alignment > 0);

        if (i_alignment <= detail::MaxAlignment && i_alignment_offset == 0)
        {
#if __cpp_sized_deallocation >= 201309
            operator delete(i_block, i_size); // since C++14
#else
            (void)i_size;
            operator delete(i_block);
#endif
        }
        else
        {
            if (i_block != nullptr)
            {
                const auto & header = *(static_cast<detail::AlignmentHeader *>(i_block) - 1);
#if __cpp_sized_deallocation >= 201309 // since C++14
                size_t const extra_size =
                  Max(i_alignment, sizeof(detail::AlignmentHeader));
                size_t const actual_size = i_size + extra_size;
                             operator delete(header.m_block, actual_size);
#else
                (void)i_size;
                operator delete(header.m_block);
#endif
            }
        }
    }

} // namespace core
