
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <stdint.h>
#include <deque>
#include <type_traits>

namespace djup
{
    /** Queue that provides integer progressive handles for elements.
       UINT_HANDLE_TYPE should be an unsigned integer large enough to avoid aliasing.
       The handle value zero is never used and can be used as invalid value.
       The current implementation is based on std::deque.
       To do: replace std::deque with a vector-like queue.
       To do: fix overflow management. */
    template <typename ELEMENT_TYPE, typename UINT_HANDLE_TYPE = uint64_t>
        class Queue
    {
    public:

        static_assert(std::is_integral_v<UINT_HANDLE_TYPE> && std::is_unsigned_v<UINT_HANDLE_TYPE>);

        using ElementType = ELEMENT_TYPE;
        using HandleType = UINT_HANDLE_TYPE;

        Queue() = default;

        Queue(const Queue &) = delete;

        Queue & operator = (const Queue &) = delete;

        template <typename... CONSTRUCTION_PARAMS>
            HandleType Push(CONSTRUCTION_PARAMS &&... i_params)
        {
            HandleType new_handle = m_base_index + static_cast<HandleType>(m_elements.size());
            m_elements.emplace_back(std::forward<CONSTRUCTION_PARAMS>(i_params)...);
            return new_handle;
        }

        ElementType & GetFirst()
        {
            return m_elements.front();
        }

        const ElementType & GetFirst() const
        {
            return m_elements.front();
        }

        ElementType * GetByHandle(HandleType i_handle)
        {
            HandleType end_handle = m_base_index + static_cast<HandleType>(m_elements.size());
            if(end_handle < m_base_index) // [[unlikely]]
            {
                // overflow case
                if(i_handle < m_base_index)
                    return nullptr;
            }
            else
            {
                if(i_handle < m_base_index || i_handle >= end_handle)
                    return nullptr;
            }

            return &m_elements[static_cast<size_t>(i_handle - m_base_index)];
        }

        const ElementType * GetByHandle(HandleType i_handle) const
        {
            HandleType end_handle = m_base_index + static_cast<HandleType>(m_elements.size());
            if(i_handle < m_base_index || i_handle >= end_handle)
                return nullptr;
            else
                return &m_elements[static_cast<size_t>(i_handle - m_base_index)];
        }

        void Pop() noexcept
        {
            m_elements.pop_front();
            m_base_index++;
        }

        ElementType GetAndPop() noexcept
        {
            static_assert(std::is_nothrow_move_constructible_v<ElementType>);

            ElementType element = std::move(m_elements.front());
            m_elements.pop_front();
            m_base_index++;
            return element;
        }

        using iterator = typename std::deque<ElementType>::iterator;
        using const_iterator = typename std::deque<ElementType>::const_iterator;

        iterator begin() noexcept { return m_elements.begin(); }
        iterator end() noexcept{ return m_elements.end(); }

        const_iterator begin() const noexcept{ return m_elements.begin(); }
        const_iterator end() const noexcept { return m_elements.end(); }

        const_iterator cbegin() const noexcept{ return m_elements.begin(); }
        const_iterator cend() const noexcept { return m_elements.end(); }

        size_t size() const noexcept
        {
            return m_elements.size();
        }

        bool empty() const noexcept
        {
            return m_elements.empty();
        }

    private:
        HandleType m_base_index = 1;
        std::deque<ElementType> m_elements;
    };

} // namespace djup
