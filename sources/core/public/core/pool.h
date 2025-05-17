
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <vector>
#include <type_traits>
#include <limits>
#include <iterator>
#include <cstring>

namespace core
{
    /** Grow-able fixed-size pool of objects. Object allocation and 
        deallocation is very fast (with constant time), as a free-list is 
        stored inside non-allocated slots. Anyway with a random allocation/
        deallocation sequence the storage of the pool becomes fragmented.
        References to object can be stored by handles and converted to 
        a direct pointer by the pool. Direct pointers and references to 
        objects are invalidated by allocation and deletion, while handles and
        iterators are not. The pool can be queried whether an handler is dangling,
        that is its object has been deallocated, because a version number is 
        stored in both the handle and the object, so that if the object is 
        destroyed and the memory space is reused for another object, the pool 
        can detect that the handle is no more valid.
        The UINT parameter is the type used for the version number. Since it
        is incremented when the object is destroyed and it can wrap to zero,
        there is an extremely low probability that an handle may refer to a
        more recent object, while the original object has been destroyed a 
        long time ago.
        The template argument UINT is used for indices, sizes, and version 
        number The default is size_t. It's not recommended to use less than 
        32 bits. UINT must be an unsigned integer type.

    Item layout with 64-bit UINT:
    ----------------------------------------- - - - - --
    |           |               | next-free            |
    |           |               | (64 bits)            |
    |  version  | is_allocated  | --- or ------        |
    | (63 bits) |   (1 bit)     | element              |
    |           |               | (n-bits)             |
    ---------------------------------------- - - - - --|            
    */  
    template <typename ELEMENT, typename UINT = size_t>
        class Pool
    {
    public:

        static_assert(std::is_integral_v<UINT> && std::is_unsigned_v<UINT> &&
            std::numeric_limits<UINT>::radix == 2, 
            "UINT must be an unsigned base-2 integer");

        using Element = ELEMENT;
        using UInt = UINT;

        /** Handle to an object allocated in the pool. THe member function IsValid 
            and TryGetObject can be used to check if the object is still allocated, 
            whether or not its location has been reused for another allocation.
            Default-initialized handles have null-like value, and their always result
            invalid. They can be instantiated with Handle{ }. 
            Equality and disequality operators are provided. */
        struct Handle;

        /** Constructs an empty pool with a default initial capacity */
        Pool();

        /** Constructs an empty pool with the specified initial capacity */
        Pool(UINT i_initial_capacity);

        // copy and move are technically possible, but currently not provided
        Pool(const Pool&) = delete;
        Pool& operator=(const Pool&) = delete;

        /** Destroys the pool. All objects must be deallocated, otherwise the
            behavior is undefined */
        ~Pool();

        /** Creates a new object. The returned handle can be accessed with 
            GetObject or TryGetObject. Any pointer/reference to an element is 
            invalidated, and must be refreshed by GetObject/TryGetObject. 
            Iterators are not invalidated. */
        template <typename... ARGS>
            Handle New(ARGS &&... i_args);

        /** Destroys an object. The handle become dangling, and calling 
            IsValid on it will return false. If the object has already 
            been deleted or the handle is null the behavior is 
            undefined. Any pointer/reference to an element is 
            invalidated, and must be refreshed by GetObject/TryGetObject. 
            Iterators are not invalidated. */
        void Delete(Handle i_handle);

        /** Returns whether an object is still allocated in the pool,
            false if it was deleted or is a null handle. */
        bool IsValid(Handle i_handle) const;

        /** Returns a direct reference to an object allocated in the pool.
            If the object ha been deallocated or is a null handle the 
            behavior is undefined.
            The reference is valid as long as the pool is not altered.
            Even creating a new object of the pool invalidating the reference,
            so use direct references only locally, and keep the handle to 
            refer to the object. */
        ELEMENT & GetObject(Handle i_handle);

        /** Const overload of GetObject. */
        const ELEMENT & GetObject(Handle i_handle) const;

        /** Returns a direct pointer to an object allocated in the pool,
            or nullptr if the object has been deallocated or is a null handle.
            The pointer is valid as long as the pool is not altered.
            Even creating a new object of the pool invalidating the pointer,
            so use direct pointers only locally, and keep the handle to 
            refer to the object. */
        ELEMENT * TryGetObject(Handle i_handle);

        /** Const overload of TryGetObject. */
        const ELEMENT * TryGetObject(Handle i_handle) const;

        /** Returns the handle of a given object in the pool. If the object
            is not present in the pool, or it has been deleted, the behavior
            is undefined. A null handle is never returned. */
        Handle HandleOf(const ELEMENT & i_element) const;

        /** Returns the currently allocated object count. */
        UINT GetObjectCount() const;

        /** Forward iteration is linear in the capacity of the pool, not in the number
            of allocated objects. */
        class Iterator;
        Iterator begin();
        Iterator end();

        /** Forward iteration is linear in the capacity of the pool, not in the number 
            of allocated objects. */
        class ConstIterator;
        ConstIterator cbegin() const;
        ConstIterator cend() const;
        ConstIterator begin() const;
        ConstIterator end() const;

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