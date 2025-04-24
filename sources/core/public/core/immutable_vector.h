
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <core/memory.h>
#include <core/traits.h>
#include <utility>
#include <iterator>
#include <assert.h>

namespace core
{
    /** Class template for immutable shared vectors. Copying is cheap as the
        elements of the vector are shared (a ref-count is allocated at the
        beginning of the memory block). */
    template <typename ELEMENT>
        class ImmutableVector
    {
    public:

        using iterator_category = std::random_access_iterator_tag; // could be contiguous in C++20
        using value_type = ELEMENT;
        using difference_type = ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;

        ImmutableVector() noexcept
            : m_elements(GetEmptyElements()), m_size(0)
        {
            ++s_empty_header.m_ref_count;
        }

        ImmutableVector(const ImmutableVector & i_source) noexcept
            : m_elements(i_source.m_elements), m_size(i_source.m_size)
        {
            Header & header = i_source.GetHeader();
            ++header.m_ref_count;
        }

        ImmutableVector(ImmutableVector && i_source) noexcept
            : m_elements(i_source.m_elements), m_size(i_source.m_size)
        {
            i_source.m_elements = GetEmptyElements();
            i_source.m_size = 0;
            ++s_empty_header.m_ref_count;
        }

        template <typename INPUT_ITERATOR, 
                typename = typename std::iterator_traits<INPUT_ITERATOR>::difference_type,
                typename = typename std::iterator_traits<INPUT_ITERATOR>::pointer,
                typename = typename std::iterator_traits<INPUT_ITERATOR>::reference,
                typename = typename std::iterator_traits<INPUT_ITERATOR>::value_type,
                typename = typename std::iterator_traits<INPUT_ITERATOR>::iterator_category>
            ImmutableVector(INPUT_ITERATOR i_begin, INPUT_ITERATOR i_end)
        {
            Allocate(i_end - i_begin);

            ELEMENT* dest = m_elements;
            for (INPUT_ITERATOR curr = i_begin; curr != i_end; ++curr, ++dest)
            {
                new (dest) ELEMENT(*curr);
            }
        }

        ImmutableVector(std::initializer_list<char> i_initializer)
        {
            Allocate(i_initializer.size());

            ELEMENT* dest = m_elements;
            for (const char * curr = i_initializer.begin();
                curr != i_initializer.end(); ++curr, ++dest)
            {
                new (dest) ELEMENT(*curr);
            }
        }

        ImmutableVector& operator = (ImmutableVector i_source) noexcept
        {
            swap(*this, i_source);
            return *this;
        }

        friend void swap(ImmutableVector& i_first, ImmutableVector& i_second) noexcept
        {
            using std::swap;
            swap(i_first.m_elements, i_second.m_elements);
            swap(i_first.m_size, i_second.m_size);
        }

        const ELEMENT * data() const noexcept
        {
            return m_elements;
        }

        size_t size() const noexcept
        {
            return m_size;
        }

        const ELEMENT & operator [] (size_t i_index) const noexcept
        {
            assert(i_index < m_size);
            return m_elements[i_index];
        }

        bool empty() const noexcept { return m_size == 0; }

        ~ImmutableVector()
        {
            Header & header = GetHeader();
            assert(header.m_ref_count > 0);
            --header.m_ref_count;
            if (header.m_ref_count == 0 && &header != &s_empty_header)
            {
                for (size_t i = 0; i < m_size; i++)
                    m_elements[i].ELEMENT::~ELEMENT();

                aligned_deallocate(&header, m_size * sizeof(ELEMENT) + sizeof(Header),
                    alignof(ELEMENT), sizeof(Header));
            }
        }

        class ConstIterator
        {
        public:

            using iterator_category = std::random_access_iterator_tag; // could be contiguous in C++20
            using value_type = ELEMENT;
            using difference_type = ptrdiff_t;
            using pointer = value_type*;
            using reference = value_type&;

            ConstIterator(ELEMENT* i_curr) noexcept : m_curr(i_curr) {}

            const ELEMENT & operator * () const noexcept { return *m_curr; }

            const ELEMENT * operator -> () const noexcept { return m_curr; }

            ConstIterator& operator ++ () noexcept
            {
                ++m_curr;
                return *this;
            }

            ConstIterator operator ++ (int) noexcept
            {
                auto copy = *this;
                ++m_curr;
                return copy;
            }

            bool operator == (const ConstIterator& i_other) const noexcept
            {
                return m_curr == i_other.m_curr;
            }

            bool operator != (const ConstIterator& i_other) const noexcept
            {
                return m_curr != i_other.m_curr;
            }

            difference_type operator - (const ConstIterator& i_other) const noexcept
            {
                return m_curr - i_other.m_curr;
            }
            
        private:
            ELEMENT* m_curr{};
        };

        ConstIterator cbegin() const noexcept { return ConstIterator(m_elements); }
        ConstIterator cend() const noexcept { return ConstIterator(m_elements + m_size); }

        ConstIterator begin() const noexcept { return ConstIterator(m_elements); }
        ConstIterator end() const noexcept { return ConstIterator(m_elements + m_size); }

    private:

        struct Header
        {
            uint32_t m_ref_count{};
        };

        Header & GetHeader() const
        {
            Header * header = reinterpret_cast<Header*>(m_elements) - 1;
            return *header;
        }

        static ELEMENT * GetEmptyElements()
        {
            return reinterpret_cast<ELEMENT*>(&s_empty_header + 1);
        }

        void Allocate(size_t i_size)
        {
            auto header = static_cast<Header*>(aligned_allocate(
                i_size * sizeof(ELEMENT) + sizeof(Header),
                alignof(ELEMENT), sizeof(Header)));;

            m_elements = reinterpret_cast<ELEMENT*>(header + 1);
            m_size = i_size;
            GetHeader().m_ref_count = 1;
        }

    private:
        ELEMENT * m_elements;
        size_t m_size;
        static Header s_empty_header;
    };

    template <typename ELEMENT>
        typename ImmutableVector<ELEMENT>::Header ImmutableVector<ELEMENT>::s_empty_header;
}
 