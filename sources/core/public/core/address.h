
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <limits>

namespace djup
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
        assert(i_alignment > 0 && is_power_of_2(i_alignment));
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
        assert(i_alignment > 0 && is_power_of_2(i_alignment));
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
        assert(i_alignment > 0 && is_power_of_2(i_alignment));

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
        assert(i_alignment > 0 && is_power_of_2(i_alignment));

        const uintptr_t uint_pointer = reinterpret_cast<uintptr_t>(i_address);

        const size_t mask = i_alignment - 1;

        return reinterpret_cast<void *>(uint_pointer & ~mask);
    }

    /** Returns the biggest address lesser than the first parameter, such that i_address + i_alignment_offset is aligned
        @param i_address address to be aligned
        @param i_alignment alignment required from the pointer. It must be an integer power of 2
        @param i_alignment_offset alignment offset
        @return the result address */
    inline void * address_lower_align(void * i_address, size_t i_alignment, size_t i_alignment_offset) noexcept
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
    inline const void * address_lower_align(const void * i_address, 
        size_t i_alignment, size_t i_alignment_offset) noexcept
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
        assert(i_alignment > 0 && is_power_of_2(i_alignment));

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
        assert(i_alignment > 0 && is_power_of_2(i_alignment));

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
    inline void * address_upper_align(void * i_address, size_t i_alignment, size_t i_alignment_offset) noexcept
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
    inline const void * address_upper_align(const void * i_address, 
        size_t i_alignment, size_t i_alignment_offset) noexcept
    {
        const void * address = address_add(i_address, i_alignment_offset);

        address = address_upper_align(address, i_alignment);

        address = address_sub(address, i_alignment_offset);

        return address;
    }

    #define SNAB_NO_UNIQUE_ADDRESS // [[no_unique_address]]
}
