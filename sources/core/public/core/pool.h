
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <vector>
#include <type_traits>
#include <limits>

namespace core
{
    /** Grow-able fixed-size pool of objects.
     
    
    Implementation details
    ------------------------------------------
    |           |               | next-free  |
    |           |               | (64 bits)  |
    |  version  | is_allocated  | ---------  |
    | (63 bits) |   (1 bit)     | element    |
    |           |               | (n-bits)   |
    |-----------|---------------|------------|
    |           |               |            |
    |-----------|---------------|------------|
    |           |               |            |
    |-----------|---------------|------------|
    |           |               |            |
    |-----------|---------------|------------|
    |           |               |            |
    |-----------|---------------|------------|
    |           |               |            |
    |-----------|---------------|------------|
    |           |               |            |
    |-----------|---------------|------------|
    |           |               |            |
    |-----------|---------------|------------|
    |           |               |            |
    |-----------|---------------|------------|
    
    */  
    template <typename ELEMENT, typename UINT = size_t>
        class Pool
    {
    public:

        static_assert(std::is_integral_v<UINT> && std::is_unsigned_v<UINT> &&
            std::numeric_limits<UINT>::radix == 2, "UINT must be an unsigned base-2 integer");

        struct Handle
        {
            UINT m_index, m_version;
        };

        Pool();

        Pool(UINT i_initial_capacity);

        Pool(const Pool&) = delete;

        Pool& operator=(const Pool&) = delete;

        ~Pool();

        template <typename... ARGS>
            Handle New(ARGS &&... i_args)
        {
            const Handle handle = Allocate();
            new(&m_items[handle.m_index].m_element)
                ELEMENT(std::forward<ARGS>(i_args)...);
            return handle;
        }

        void Delete(Handle i_handle);

        bool IsValid(Handle i_handle) const;

        ELEMENT & GetObject(Handle i_handle);

        ELEMENT * TryGetObject(Handle i_handle);

        UINT GetObjectCount() const;

        size_t WalkFreeList() const;

    private:
        
        Handle Allocate();

        Handle AllocateSlowPath();

        void Deallocate(Handle i_handle);

        struct Item;

    private:
        std::vector<Item> m_items;
        UINT m_first_free_index;
        UINT m_allocated_objects;
    };
}

#include <core/pool.inl> 