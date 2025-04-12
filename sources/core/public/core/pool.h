
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
    /** Grow-able fixed-size pool of objects. References to object can be stored 
        by handles and converted to a direct pointer by the pool. The pool can
        be queried whether an handler is dangling.
     
    
    Item layout:
    ------------------------------------------
    |           |               | next-free  |
    |           |               | (64 bits)  |
    |  version  | is_allocated  | ---------  |
    | (63 bits) |   (1 bit)     | element    |
    |           |               | (n-bits)   |
    |-----------|---------------|------------|
    
    */  
    template <typename ELEMENT, typename UINT = size_t>
        class Pool
    {
    public:

        static_assert(std::is_integral_v<UINT> && std::is_unsigned_v<UINT> &&
            std::numeric_limits<UINT>::radix == 2, "UINT must be an unsigned base-2 integer");

        using Element = ELEMENT;
        using UInt = UINT;

        /** Handle to an object allocated in the pool. THe member function IsValid 
            and TryGetObject can be used to check if the object is still allocated, 
            whether or not its location has been reused for another allocation.
            Uninitialized handles have undefined, and their use with a pool causes
            undefined behavior. */
        struct Handle;

        /** Constructs an empty pool with a default initial capacity */
        Pool();

        /** Constructs an empty pool with the specified initial capacity */
        Pool(UINT i_initial_capacity);

        // move is technically possible, but currently is not provided
        Pool(const Pool&) = delete;
        Pool& operator=(const Pool&) = delete;

        /** Constructs an empty pool with the specified initial capacity */
        ~Pool();

        /** Creates a new object. The returned handle can be converted 
            to a direct pointer with GetObject or TryGetObject */
        template <typename... ARGS>
            Handle New(ARGS &&... i_args);

        /** Destroys an object. The handle become dangling, and calling 
            IsValid on it will return false. */
        void Delete(Handle i_handle);

        /** Returns whether an object is still allocated in the pool. */
        bool IsValid(Handle i_handle) const;

        /** Returns a direct pointer to an object allocated in the pool.
            If the object ha been deallocated the behavior is undefined. */
        ELEMENT & GetObject(Handle i_handle);

        /** Returns a direct pointer to an object allocated in the pool,
            or nullptr if the object has been deallocated */
        ELEMENT * TryGetObject(Handle i_handle);

        UINT GetObjectCount() const;

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