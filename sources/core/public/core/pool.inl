
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
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
    // handle
    template <typename ELEMENT, typename UINT>
        struct Pool<ELEMENT, UINT>::Handle
    {
        UINT m_index = ~static_cast<UINT>(0);
        UINT m_version = ~static_cast<UINT>(0);

        bool operator == (const typename Pool<ELEMENT, UINT>::Handle& i_second) const
        {
            return m_index == i_second.m_index &&
                m_version == i_second.m_version;
        }

        bool operator != (const typename Pool<ELEMENT, UINT>::Handle& i_second) const
        {
            return m_index != i_second.m_index ||
                m_version != i_second.m_version;
        }
    };

    // Item
    template <typename ELEMENT, typename UINT>
        struct Pool<ELEMENT, UINT>::Item
    {

    private:
        
        /* the version is used to detect if the slot has been recycled by 
           a more recent slot. The is_allocated is necessary when the internal
           vector reallocates after exceeding capacity, to know if the slot 
           contains an object or a link of the free list. */
        
        // UINT m_version : (std::numeric_limits<UINT>::digits - 1);
        // UINT m_is_allocated : 1;
        
        /** Apparently Visual Studio 17.13.4 has a bug that wraps to 0
            a 63-bit field when incrementing it from 1, so we handle
            the bits by hand. I was not able to reproduce a small instance
            of this problem, and reducing this one may be time consuming. */
        UINT m_version__is_allocated;
        constexpr static UINT s_version_mask = bits<UINT>(0, std::numeric_limits<UINT>::digits - 1);
        constexpr static UINT s_is_allocated_mask = bit_reverse<UINT>(0);

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
        
        /* this is used when the internal vector is relocated
           because of capacity exhaustion */
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

        /* this is used when the internal vector is relocated
           because of capacity exhaustion */
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
            the pool must be empty: non-deallocated object won't
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
        assert(IsValid(i_handle));

        Item & item = m_items[i_handle.m_index];      

        // the newly deallocated item has as next the former first free
        item.m_next_free = m_first_free_index;
        // the new first free is the newly deallocated item
        m_first_free_index = i_handle.m_index;

        item.IncrementVersion();
        item.SetIsAllocated(false);
        #ifndef NDEBUG
            /* write garbage on the element memory. The m_next_free
               field must be saved and restored because m_element and
               m_next_free overlap. */
            const UINT next_free = item.m_next_free;
            memset(&item.m_element, 0Xcd, sizeof(item.m_element));
            item.m_next_free = next_free;
        #endif
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
        /* The last bit of the version is used as 'is_allocated' flag,
            so the version can never be so high. */
        if (~i_handle.m_version == 0)
            return false;

        const Item& item = m_items[i_handle.m_index];
        return i_handle.m_version == item.GetVersion();
    }

    template <typename ELEMENT, typename UINT>
        ELEMENT & Pool<ELEMENT, UINT>::GetObject(Handle i_handle)
    {
        Item& item = m_items[i_handle.m_index];
        assert(IsValid(i_handle) && item.IsAllocated());
        assert(i_handle.m_version == item.GetVersion()); // invalid object access
        return item.m_element;
    }

    template <typename ELEMENT, typename UINT>
        const ELEMENT & Pool<ELEMENT, UINT>::GetObject(Handle i_handle) const
    {
        const Item& item = m_items[i_handle.m_index];
        assert(IsValid(i_handle) && item.IsAllocated());
        assert(i_handle.m_version == item.GetVersion()); // invalid object access
        return item.m_element;
    }

    template <typename ELEMENT, typename UINT>
        ELEMENT * Pool<ELEMENT, UINT>::TryGetObject(Handle i_handle)
    {
        if (!IsValid(i_handle))
            return nullptr;

        Item& item = m_items[i_handle.m_index];
        if (i_handle.m_version == item.GetVersion() && IsValid(i_handle))
        {
            assert(item.IsAllocated());
            return &item.m_element;
        }
        else
            return nullptr;
    }

    template <typename ELEMENT, typename UINT>
        const ELEMENT * Pool<ELEMENT, UINT>::TryGetObject(Handle i_handle) const
    {
        if (!IsValid(i_handle))
            return nullptr;

        const Item & item = m_items[i_handle.m_index];
        if (i_handle.m_version == item.GetVersion() && IsValid(i_handle))
        {
            assert(item.IsAllocated());
            return &item.m_element;
        }
        else
            return nullptr;
    }

    template <typename ELEMENT, typename UINT>
        typename Pool<ELEMENT, UINT>::Handle
            Pool<ELEMENT, UINT>::HandleOf(const ELEMENT& i_element) const
    {
        auto const begin = reinterpret_cast<const char*>(m_items.data());
        auto const end = reinterpret_cast<const char*>(m_items.data() + m_items.size());
        auto const ptr = reinterpret_cast<const char*>(&i_element);
        
        assert(ptr >= begin && ptr < end);
        
        UINT const byte_offset = NumericCast<UINT>(ptr - begin);
        UINT const index = byte_offset / sizeof(Item);
        return { index, m_items[index].GetVersion() };
    }

    template <typename ELEMENT, typename UINT>
        UINT Pool<ELEMENT, UINT>::GetObjectCount() const
    {
        return m_allocated_objects;
    }

    // Iterator
    template <typename ELEMENT, typename UINT>
        class Pool<ELEMENT, UINT>::Iterator
    {
    public:

        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::make_signed_t<UINT>;
        using value_type = ELEMENT;
        using pointer = ELEMENT*;
        using reference = ELEMENT&;

        enum class BeginTag {};
        enum class EndTag {};

        Iterator(Pool& i_pool, BeginTag)
            : m_pool(i_pool), m_index(0) 
        {
            SkipSlotsNotAllocated();
        }

        Iterator(Pool& i_pool, EndTag)
            : m_pool(i_pool), m_index(NumericCast<UINT>(i_pool.m_items.size()))
        {
            
        }

        reference operator*() const { return m_pool.m_items[m_index].m_element; }

        pointer operator -> () { return &m_pool.m_items[m_index].m_element; }

        Iterator & operator ++() 
        { 
            ++m_index; 
            SkipSlotsNotAllocated();
            return *this; 
        }

        Iterator operator ++(int)
        {
            const Iterator result = *this;
            ++m_index;
            SkipSlotsNotAllocated();
            return result;
        }

        bool operator == (const Iterator& i_second) const
        {
            return &m_pool == &i_second.m_pool &&
                m_index == i_second.m_index;
        }

        bool operator != (const Iterator& i_second) const
        {
            return &m_pool != &i_second.m_pool ||
                m_index != i_second.m_index;
        }

    private:

        void SkipSlotsNotAllocated()
        {
            while (m_index < m_pool.m_items.size() &&
                !m_pool.m_items[m_index].IsAllocated() )
            {
                ++m_index;
            }
        }

    private:
        Pool & m_pool;
        UINT m_index;
    };

    template <typename ELEMENT, typename UINT>
        typename Pool<ELEMENT, UINT>::Iterator
            Pool<ELEMENT, UINT>::begin()
    {
        return Iterator( *this, typename Iterator::BeginTag() );
    }

    template <typename ELEMENT, typename UINT>
        typename Pool<ELEMENT, UINT>::Iterator
            Pool<ELEMENT, UINT>::end()
    {
        return Iterator( *this, typename Iterator::EndTag() );
    }


    // ConstIterator
    template <typename ELEMENT, typename UINT>
        class Pool<ELEMENT, UINT>::ConstIterator
    {
    public:

        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::make_signed_t<UINT>;
        using value_type = const ELEMENT;
        using pointer = const ELEMENT*;
        using reference = const ELEMENT&;

        enum class BeginTag {};
        enum class EndTag {};

        ConstIterator(const Pool& i_pool, BeginTag)
            : m_pool(i_pool), m_index(0) 
        {
            SkipSlotsNotAllocated();
        }

        ConstIterator(const Pool& i_pool, EndTag)
            : m_pool(i_pool), m_index(NumericCast<UINT>(i_pool.m_items.size()))
        {
        }

        reference operator*() const { return m_pool.m_items[m_index].m_element; }

        pointer operator -> () { return &m_pool.m_items[m_index].m_element; }

        ConstIterator & operator ++() 
        { 
            ++m_index; 
            SkipSlotsNotAllocated();
            return *this; 
        }

        ConstIterator operator ++(int)
        {
            const ConstIterator result = *this;
            ++m_index;
            SkipSlotsNotAllocated();
            return result;
        }

        bool operator == (const ConstIterator& i_second) const
        {
            return &m_pool == &i_second.m_pool &&
                m_index == i_second.m_index;
        }

        bool operator != (const ConstIterator& i_second) const
        {
            return &m_pool != &i_second.m_pool ||
                m_index != i_second.m_index;
        }

    private:

        void SkipSlotsNotAllocated()
        {
            while (m_index < m_pool.m_items.size() &&
                !m_pool.m_items[m_index].IsAllocated() )
            {
                ++m_index;
            }
        }

    private:
        const Pool & m_pool;
        UINT m_index;
    };

    template <typename ELEMENT, typename UINT>
        typename Pool<ELEMENT, UINT>::ConstIterator
            Pool<ELEMENT, UINT>::begin() const
    {
        return ConstIterator( *this, typename ConstIterator::BeginTag() );
    }

    template <typename ELEMENT, typename UINT>
        typename Pool<ELEMENT, UINT>::ConstIterator
            Pool<ELEMENT, UINT>::end() const
    {
        return ConstIterator(*this, typename ConstIterator::EndTag());
    }

    template <typename ELEMENT, typename UINT>
        typename Pool<ELEMENT, UINT>::ConstIterator
            Pool<ELEMENT, UINT>::cbegin() const
    {
        return ConstIterator( *this, typename ConstIterator::BeginTag() );
    }

    template <typename ELEMENT, typename UINT>
        typename Pool<ELEMENT, UINT>::ConstIterator
            Pool<ELEMENT, UINT>::cend() const
    {
        return ConstIterator( *this, typename ConstIterator::EndTag() );
    }

} // namespace core
