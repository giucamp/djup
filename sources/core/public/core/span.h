
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <iterator>
#include <utility>
#include <type_traits>
#include <algorithm>
#include <stdexcept>
#include <assert.h>
#include <core/pointer_iterator.h>
#include <core/traits.h>

namespace djup
{
    template <typename TYPE>
        class Span
    {
    public:

        using value_type = TYPE;
        using reference = TYPE&;
        using pointer = TYPE*;
        using const_reference = const TYPE&;
        using difference_type = ptrdiff_t;
        using size_type = size_t;
        using iterator = PointerIterator<TYPE>;
        using const_iterator = PointerIterator<const TYPE>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        constexpr Span() = default;

        constexpr Span(TYPE * i_data, size_t i_size)
            : m_data(i_data), m_size(i_size) { }

        template <typename SOURCE_CONTAINER, typename = std::enable_if_t<IsContigousContainerOfV<SOURCE_CONTAINER, TYPE>>>
            constexpr Span(SOURCE_CONTAINER && i_source_container)
                : m_data(std::data(i_source_container)),
                  m_size(std::size(i_source_container))
                    { }

        template <typename SOURCE_CONTAINER, typename = std::enable_if_t<IsContigousContainerOfV<SOURCE_CONTAINER, TYPE>>>
            constexpr Span(SOURCE_CONTAINER && i_source_container,
                size_t i_span_offset, size_t i_span_size = std::numeric_limits<size_t>::max())
        {
            const auto source_data = std::data(i_source_container);
            const auto source_size = std::size(i_source_container);

            LIQUID_ASSERT(i_span_offset <= source_size);
            size_t const max_span_size = source_size - i_span_offset;
            auto const span_data = source_data + i_span_offset;
            *this = {span_data, span_data + std::min(i_span_size, max_span_size) };
        }

        constexpr Span(std::initializer_list<TYPE> && i_initializer_list)
            : m_data(i_initializer_list.begin()),
              m_size(i_initializer_list.size())
                { }

        constexpr bool empty() const { return m_size == 0; }

        constexpr TYPE * data() const { return m_data; }

        constexpr size_t size() const { return m_size; }

        constexpr const TYPE & operator[](size_t i_index) const
        {
            assert(i_index < m_size);
            return m_data[i_index];
        }

        constexpr TYPE & operator[](size_t i_index)
        {
            assert(i_index < m_size);
            return m_data[i_index];
        }

        constexpr const TYPE & at(size_t i_index) const
        {
            if(i_index >= m_size)
                throw std::out_of_range("Span - out of range");
            return m_data[i_index];
        }

        constexpr TYPE & at(size_t i_index)
        {
            if(i_index >= m_size)
                throw std::out_of_range("Span - out of range");
            return m_data[i_index];
        }

        constexpr TYPE & front()
        {
            assert(m_size > 0);
            return m_data[0];
        }

        constexpr const TYPE & front() const
        {
            assert(m_size > 0);
            return m_data[0];
        }

        constexpr TYPE & back()
        {
            assert(m_size > 0);
            return m_data[m_size - 1];
        }

        constexpr const TYPE & back() const
        {
            assert(m_size > 0);
            return m_data[m_size - 1];
        }

        constexpr Span subspan(size_t i_offset, size_t i_size) const
        {
            assert(i_offset <= m_size && i_offset + i_size <= m_size);
            return {m_data + i_offset, i_size};
        }

        constexpr Span subspan(size_t i_offset) const
        {
            return subspan(i_offset, m_size - i_offset);
        }

        constexpr bool content_equals(Span<const TYPE> i_other) const
        {
            return m_size == i_other.size() &&
                std::equal(begin(), end(), i_other.begin());
        }

        constexpr iterator begin() { return iterator{ m_data }; }
        constexpr const_iterator begin() const { return const_iterator{ m_data }; }
        constexpr const_iterator cbegin() const { return const_iterator{ m_data }; }
        constexpr reverse_iterator rbegin() { return std::make_reverse_iterator(end()); }
        constexpr const_reverse_iterator rbegin() const { return std::make_reverse_iterator(end()); }
        constexpr const_reverse_iterator crbegin() const { return std::make_reverse_iterator(end()); }

        constexpr iterator end() { return iterator{ m_data + m_size }; }
        const_iterator end() const { return const_iterator{ m_data + m_size }; }
        const_iterator cend() const { return const_iterator{ m_data + m_size }; }
        reverse_iterator rend() { return std::make_reverse_iterator(begin()); }
        const_reverse_iterator rend() const { return std::make_reverse_iterator(begin()); }
        const_reverse_iterator crend() const { return std::make_reverse_iterator(begin()); }

    private:
        TYPE * m_data{};
        size_t m_size{};
    };

    // template argument deduction guides for Span
    template <typename SOURCE_CONTAINER>
        Span(SOURCE_CONTAINER & i_source_container) -> Span<ContainerElementTypeT<SOURCE_CONTAINER>>;
    template <typename SOURCE_CONTAINER>
        Span(const SOURCE_CONTAINER & i_source_container) -> Span<const ContainerElementTypeT<SOURCE_CONTAINER>>;
    template <typename ELEMENT_TYPE>
        Span(std::initializer_list<ELEMENT_TYPE> i_source_container) -> Span<const ELEMENT_TYPE>;
}
