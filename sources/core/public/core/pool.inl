
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <core/pool.h>
#include <core/diagnostic.h>
#include <core/numeric_cast.h>
#include <core/bits.h>
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
        /** Apparently Visual Studio 17.13.4 has a bug that wraps to 0 
            a 63-bit field when incrementing it from 1, so we handle 
            the bits by hand. */
    private:
        // UINT m_version : (std::numeric_limits<UINT>::digits - 1);
        // UINT m_is_allocated : 1;

        constexpr static UINT s_version_mask = bits<UINT>(0, std::numeric_limits<UINT>::digits - 1);
        constexpr static UINT s_is_allocated_mask = bit_reverse<UINT>(0);
        
        UINT m_version__is_allocated;
    
    public:
        UINT GetVersion() const { return m_version__is_allocated & s_version_mask; }

        bool IsAllocated() const { return (m_version__is_allocated & s_is_allocated_mask) != 0; }

        void SetVersion(UINT i_version)
        {
            UINT alloc = m_version__is_allocated & s_is_allocated_mask;
            m_version__is_allocated = i_version & s_version_mask;
            m_version__is_allocated |= alloc;
        }

        void SetIsAllocated(bool i_is_allocated)
        {
            if (i_is_allocated)
                m_version__is_allocated |= s_is_allocated_mask;
            else
                m_version__is_allocated &= ~s_is_allocated_mask;
        }

        void SetVersionAndIsAllocated(UINT i_version, bool i_is_allocated)
        {
            m_version__is_allocated = i_version & s_version_mask;
            if (i_is_allocated)
                m_version__is_allocated |= s_is_allocated_mask;
            else
                m_version__is_allocated &= ~s_is_allocated_mask;
        }

        void IncrementVersion()
        {
            UINT alloc = m_version__is_allocated & s_is_allocated_mask;
            ++m_version__is_allocated;
            m_version__is_allocated &= s_version_mask;
            m_version__is_allocated |= alloc;
        }

    public:
        union
        {
            UINT m_next_free;
            ELEMENT m_element;
        };

        Item() 
        {
            SetVersionAndIsAllocated(0, false);
        }
        
        Item(const Item & i_source)
        {
            SetVersion(i_source.GetVersion());
            bool is_alloc = i_source.IsAllocated();
            SetIsAllocated(is_alloc);
            if (is_alloc)
            {
                new (&m_element) ELEMENT(i_source.m_element);
            }
            else
            {
                m_next_free = i_source.m_next_free;
            }
        }

        Item(Item && i_source) noexcept
        {
            SetVersion(i_source.GetVersion());
            bool is_alloc = i_source.IsAllocated();
            SetIsAllocated(is_alloc);
            if (is_alloc)
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
            if(IsAllocated())
                m_element.ELEMENT::~ELEMENT();
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
            const UINT version = item.GetVersion();
            m_first_free_index = item.m_next_free;
            ++m_allocated_objects;
            item.SetIsAllocated(true);
            return Handle{ index, version };
        }
        else
        {
            return AllocateSlowPath();
        }
    }

    // Allocate a new object - to do: try to use __declspec(noinline) and __attribute__((noinline))
    template <typename ELEMENT, typename UINT>
        typename Pool<ELEMENT, UINT>::Handle
            Pool<ELEMENT, UINT>::AllocateSlowPath()
    {
        const UINT index = NumericCast<UINT>(m_items.size());
        Item& item = m_items.emplace_back();
        item.SetVersionAndIsAllocated(0, true);
        ++m_first_free_index;
        ++m_allocated_objects;
        return { index, 0 };
    }

    // Allocates and construct a new object
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

        item.IncrementVersion();
        item.SetIsAllocated(false);
        
        assert(m_allocated_objects > 0);
        --m_allocated_objects;
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
        return i_handle.m_version == item.GetVersion();
    }

    template <typename ELEMENT, typename UINT>
        ELEMENT & Pool<ELEMENT, UINT>::GetObject(Handle i_handle)
    {
        Item& item = m_items[i_handle.m_index];
        assert(item.IsAllocated());
        assert(i_handle.m_version == item.GetVersion()); // invalid object access
        return item.m_element;
    }

    template <typename ELEMENT, typename UINT>
        ELEMENT * Pool<ELEMENT, UINT>::TryGetObject(Handle i_handle)
    {
        Item& item = m_items[i_handle.m_index];
        if (i_handle.m_version == item.GetVersion())
        {
            assert(item.IsAllocated());
            return &item.m_element;
        }
        else
            return nullptr;
    }

    template <typename ELEMENT, typename UINT>
        UINT Pool<ELEMENT, UINT>::GetObjectCount() const
    {
        return m_allocated_objects;
    }

} // namespace core
