
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <type_traits>

namespace core
{
    template <typename IT_TYPE> class PointerIterator
    {
    public:

        using value_type = IT_TYPE;
        using reference = IT_TYPE&;
        using pointer = IT_TYPE*;
        using const_reference = const IT_TYPE&;
        using difference_type = ptrdiff_t;
        using size_type = size_t;
        using iterator_category = std::random_access_iterator_tag;

        constexpr explicit PointerIterator(IT_TYPE* i_pointer)
            : m_pointer(i_pointer) {}

        constexpr bool operator == (const PointerIterator& i_other) const { return m_pointer == i_other.m_pointer; }
        constexpr bool operator != (const PointerIterator& i_other) const { return m_pointer != i_other.m_pointer; }
        constexpr bool operator > (const PointerIterator& i_other) const { return m_pointer > i_other.m_pointer; }
        constexpr bool operator >= (const PointerIterator& i_other) const { return m_pointer >= i_other.m_pointer; }
        constexpr bool operator < (const PointerIterator& i_other) const { return m_pointer < i_other.m_pointer; }
        constexpr bool operator <= (const PointerIterator& i_other) const { return m_pointer <= i_other.m_pointer; }

        constexpr PointerIterator& operator ++ () { m_pointer++; return *this; }
        constexpr PointerIterator operator ++ (int) { PointerIterator copy = *this; copy.m_pointer++; return copy; }
        constexpr PointerIterator& operator -- () { m_pointer--; return *this; }
        constexpr PointerIterator operator -- (int) { PointerIterator copy = *this; copy.m_pointer--; return copy; }

        constexpr PointerIterator& operator += (ptrdiff_t i_delta) { m_pointer += i_delta; return *this; }
        constexpr         PointerIterator& operator -= (ptrdiff_t i_delta) { m_pointer -= i_delta; return *this; }

        constexpr reference operator * () const { return *m_pointer; }
        constexpr pointer operator -> () const { return m_pointer; }
        constexpr reference operator[](size_type i_index) const { return m_pointer[i_index]; }

        constexpr friend difference_type operator - (const PointerIterator& i_first, const PointerIterator& i_second)
        {
            return i_first.m_pointer - i_second.m_pointer;
        }

        template <typename INTEGER, typename = std::enable_if_t<std::is_integral_v<INTEGER> && !std::is_same_v<INTEGER, bool>>>
            constexpr friend PointerIterator operator + (const PointerIterator& i_it, INTEGER i_offset)
        {
            return PointerIterator{ i_it.m_pointer + i_offset };
        }

        template <typename INTEGER, typename = std::enable_if_t<std::is_integral_v<INTEGER> && !std::is_same_v<INTEGER, bool>>>
            constexpr friend PointerIterator operator + (INTEGER i_offset, const PointerIterator& i_it)
        {
            return PointerIterator{ i_it.m_pointer + i_offset };
        }

    private:
        IT_TYPE* m_pointer{};
    };
}
