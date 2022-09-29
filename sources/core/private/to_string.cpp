
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <core/to_string.h>
#include <algorithm>
#include <assert.h>
#include <cstring> // for memcpy

namespace djup
{
    StringBuilder::StringBuilder(size_t i_reserved_size, std::string_view i_new_line)
    {
        m_tabs = 0;
        m_tab_pending = 0;

        m_dest.resize(i_reserved_size);
        m_writer = CharBufferView(m_dest, 0);
        
        assert(i_new_line.length() < std::size(m_new_line));
        memcpy(m_new_line, i_new_line.data(), i_new_line.length());
    }

    void StringBuilder::NewLine()
    {
        *this << m_new_line;
        m_tab_pending = 1;
    }

    void StringBuilder::WriteTabs()
    {
        m_tab_pending = false;

        const char tab_chars[] = "\t\t\t\t\t\t\t\t";
        const int32_t tab_char_count = static_cast<int32_t>(std::size(tab_chars));

        int32_t tabs = m_tabs;
        while(tabs > 0)
        {
            int32_t len = std::min(tabs, tab_char_count);
            *this << std::string_view(tab_chars, len);
            tabs -= len;
        }
    }

    const std::string & StringBuilder::ShrinkAndGetString()
    {
        const size_t used_size = m_writer.GetRequiredSize();
        m_dest.resize(used_size);
        return m_dest;
    }

    std::string StringBuilder::StealString()
    {
        const size_t used_size = m_writer.GetRequiredSize();
        m_dest.resize(used_size);
        return std::move(m_dest);
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
        std::swap(i_first.m_new_line, i_second.m_new_line);

        int32_t park = i_first.m_tabs;
        i_first.m_tabs = i_second.m_tabs;
        i_second.m_tabs = park;

        park = i_first.m_tab_pending;
        i_first.m_tab_pending = i_second.m_tab_pending;
        i_second.m_tab_pending = park;
    }

} // namespace djup
