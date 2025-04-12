
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <core/pool.h>
#include <core/diagnostic.h>
#include <core/numeric_cast.h>
#include <assert.h>
#include <new>
#include <utility>
#include <limits>

namespace core
{
    template <typename ELEMENT, typename UINT>
        struct Pool<ELEMENT, UINT>::Handle
    {
        UINT m_index, m_version;
    };

    // Item
    template <typename ELEMENT, typename UINT>
        struct Pool<ELEMENT, UINT>::Item
    {
        UINT m_version : (std::numeric_limits<UINT>::digits - 1);
        UINT m_is_allocated : 1;

        union
        {
            UINT m_next_free;
            ELEMENT m_element;
        };

        Item() 
        {
            m_version = 0;
            m_is_allocated = 0;
        }
        
        Item(const Item & i_source)
        {
            m_version = i_source.m_version;
            m_is_allocated = i_source.m_is_allocated;
            if (m_is_allocated)
            {
                new (&m_element) ELEMENT(i_source.m_element);
            }
            else
            {
                m_next_free = i_source.m_next_free;
            }
        }

        Item(Item && i_source)
        {
            m_version = i_source.m_version;
            m_is_allocated = i_source.m_is_allocated;
            if (m_is_allocated)
            {
                new (&m_element) ELEMENT(std::move(i_source.m_element));
            }
            else
            {
                m_next_free = i_source.m_next_free;
            }
        }

        ~Item()
        {
        }
    };

    // Constructor
    template <typename ELEMENT, typename UINT>
        Pool<ELEMENT, UINT>::Pool()
    {
        m_first_free_index = 0;
        m_allocated_objects = 0;
    }

    // Constructor
    template <typename ELEMENT, typename UINT>
        Pool<ELEMENT, UINT>::Pool(UINT i_initial_capacity)
    {
        m_first_free_index = 0;
        m_allocated_objects = 0;
        m_items.reserve(i_initial_capacity);
    }

    // Destructor
    template <typename ELEMENT, typename UINT>
        Pool<ELEMENT, UINT>::~Pool()
    {
        assert(m_allocated_objects == 0); /* in order to be destroyed
            the allocator must be empty: non-deallocated object won't
            be destroyed*/
    }

    // Allocate a new object
    template <typename ELEMENT, typename UINT>
        typename Pool<ELEMENT, UINT>::Handle 
            Pool<ELEMENT, UINT>::Allocate()
    {
        if (m_first_free_index < m_items.size())
        {
            // not exceeding the size of the vector
            const UINT index = m_first_free_index;
            Item & item = m_items[index];
            const UINT version = item.m_version;
            m_first_free_index = item.m_next_free;
            ++m_allocated_objects;
            return Handle{ index, version };
        }
        else
        {
            return AllocateSlowPath();
        }
    }


        /*__declspec(noinline)
            __attribute__((noinline))*/

    // Allocate a new object
    template <typename ELEMENT, typename UINT>
        typename Pool<ELEMENT, UINT>::Handle
            Pool<ELEMENT, UINT>::AllocateSlowPath()
    {
        const UINT index = NumericCast<UINT>(m_items.size());
        Item& item = m_items.emplace_back();
        item.m_version = 0;
        item.m_is_allocated = 0;
        ++m_first_free_index;
        ++m_allocated_objects;
        return { index, 0 };
    }


    template <typename ELEMENT, typename UINT>
        template <typename... ARGS>
            typename Pool<ELEMENT, UINT>::Handle Pool<ELEMENT, UINT>::New(ARGS &&... i_args)
        {
            const Handle handle = Allocate();
            new(&m_items[handle.m_index].m_element)
                ELEMENT(std::forward<ARGS>(i_args)...);
            return handle;
        }

    // Deallocate an object
    template <typename ELEMENT, typename UINT>
        void Pool<ELEMENT, UINT>::Deallocate(Handle i_handle)
    {
        Item & item = m_items[i_handle.m_index];
        

        // the newly deallocated item has as next the former first free
        item.m_next_free = m_first_free_index;
        // the new first free is the newly deallocated item
        m_first_free_index = i_handle.m_index;

        ++item.m_version;
        item.m_is_allocated = false;
        --m_allocated_objects;
        assert(m_allocated_objects >= 0);
    }

    // Delete an object
    template <typename ELEMENT, typename UINT>
        void Pool<ELEMENT, UINT>::Delete(Handle i_handle)
    {
        Item& item = m_items[i_handle.m_index];
        item.m_element.ELEMENT::~ELEMENT();
        Deallocate(i_handle);
    }

    template <typename ELEMENT, typename UINT>
        bool Pool<ELEMENT, UINT>::IsValid(Handle i_handle) const
    {
        const Item& item = m_items[i_handle.m_index];
        return i_handle.m_version == item.m_version;
    }

    template <typename ELEMENT, typename UINT>
        ELEMENT & Pool<ELEMENT, UINT>::GetObject(Handle i_handle)
    {
        Item& item = m_items[i_handle.m_index];
        assert(i_handle.m_version == item.m_version); // invalid object access
        return item.m_element;
    }

    template <typename ELEMENT, typename UINT>
        ELEMENT * Pool<ELEMENT, UINT>::TryGetObject(Handle i_handle)
    {
        Item& item = m_items[i_handle.m_index];
        if (i_handle.m_version == item.m_version)
            return item.m_element;
        else
            return nullptr;
    }

    template <typename ELEMENT, typename UINT>
        UINT Pool<ELEMENT, UINT>::GetObjectCount() const
    {
        return m_allocated_objects;
    }

} // namespace core
