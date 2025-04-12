
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
    /** Grow-able fixed-size pool of objects. Object allocation and 
        deallocation is very vast (a free-list is stored inside non-allocated
        slots. References to object can be stored by handles and converted to 
        a direct pointer by the pool. The pool can be queried whether an
        handler is dangling.
        A version number is stored in both the handle and the object, so that
        if the object is destroyed and the memory space is reused for another 
        object, the pool can detect that the handle is no more valid.
        The UINT parameter is the type used for the version number. Since it
        is incremented when the object is destroyed and it can wrap to zero,
        there is an extremely low probability that an handle may refer to a
        more recent object, while the original object has been destroyed a 
        long time ago. The default of UINT is size_t. It's not recommended to
        use less than 32 bits for UINT.
        UINT must be an unsigned integer type.

    Item layout:
    ------------------------------------------------ - - - - --
    |           |               | next-free   is_allocated     |
    |           |               | (63 bits)      (1 bit)       |
    |  version  | is_allocated  | ---------                    |
    | (63 bits) |   (1 bit)     | element                      |
    |           |               | (n-bits)                     |
    |-----------|---------------|------------------- - - - - --|
    
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

        // copy and move are technically possible, but currently not provided
        Pool(const Pool&) = delete;
        Pool& operator=(const Pool&) = delete;

        /** Destroys the pool. All object must be deallocated, otherwise the
            behavior is undefined */
        ~Pool();

        /** Creates a new object. The returned handle can be converted 
            to a direct pointer with GetObject or TryGetObject. */
        template <typename... ARGS>
            Handle New(ARGS &&... i_args);

        /** Destroys an object. The handle become dangling, and calling 
            IsValid on it will return false. */
        void Delete(Handle i_handle);

        /** Returns whether an object is still allocated in the pool. */
        bool IsValid(Handle i_handle) const;

        /** Returns a direct reference to an object allocated in the pool.
            If the object ha been deallocated the behavior is undefined.
            The reference is valid as long as the object is not deleted.
        */
        ELEMENT & GetObject(Handle i_handle);

        /** Returns a direct pointer to an object allocated in the pool,
            or nullptr if the object has been deallocated.
            THe pointer is valid as long as the object is not deleted. */
        ELEMENT * TryGetObject(Handle i_handle);

        /** Returns the currently allocated object count. */
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