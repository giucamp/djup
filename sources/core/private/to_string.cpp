
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <core/to_string.h>
#include <assert.h>

namespace djup
{
    StringBuilder::StringBuilder(size_t i_reserved_size)
    {
        m_dest.resize(i_reserved_size);
        m_writer = CharBufferView(m_dest, 0);
    }

    size_t StringBuilder::size() const noexcept
    {
        return m_writer.GetRequiredSize();
    }

    const std::string & StringBuilder::ShrinkAndGetString()
    {
        const size_t used_size = size();
        m_dest.resize(used_size);
        return m_dest;
    }

    void StringBuilder::Grow(size_t i_original_used_size)
    {
        // pick new_capacity
        size_t const required_capacity = m_writer.GetRequiredSize();
        size_t new_capacity = m_dest.size() * 2;
        if(new_capacity < required_capacity)
            new_capacity = required_capacity;

        m_dest.resize(new_capacity);
        m_writer = {m_dest.data(), new_capacity, i_original_used_size};
    }

    void swap(StringBuilder & i_first, StringBuilder & i_second) noexcept
    {
        std::swap(i_first.m_dest, i_second.m_dest);
        std::swap(i_first.m_writer, i_second.m_writer);
    }

} // namespace djup
