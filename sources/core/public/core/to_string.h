
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <core/to_chars.h>
#include <utility>
#include <string>

namespace djup
{
    class StringBuilder
    {
    public:

        StringBuilder(size_t i_reserved_size = 511, std::string_view i_new_line = "\n");

        template <typename TYPE> StringBuilder & operator<<(const TYPE & i_value)
        {
            static_assert(HasCharWriterV<TYPE>, "This type does not define a CharWriter");

            if(m_tab_pending)
                WriteTabs();

            for (;;)
            {
                size_t const original_size = m_writer.GetRequiredSize();

                m_writer << i_value;
                if (!m_writer.IsTruncated())
                    return *this;

                Grow(original_size);
            }
        }

        template <typename... TYPE> StringBuilder & Write(const TYPE &... i_objects)
        {
            (void)(*this << ... << i_objects);
            return *this;
        }

        template <typename... TYPE> StringBuilder & WriteLine(const TYPE &... i_objects)
        {
            (void)(*this << ... << i_objects);
            NewLine();
            return *this;
        }

        void NewLine();

        void Tab()
        {
            m_tabs++;
        }

        void Untab()
        {
            assert(m_tabs > 0);
            m_tabs--;
        }

        const std::string & ShrinkAndGetString();

        friend void swap(StringBuilder & i_first, StringBuilder & i_second) noexcept;

        const char * GetData() const { return m_writer.data(); }

        size_t GetSize() const { return m_writer.GetRequiredSize(); }

    private:
        void Grow(size_t i_original_used_size);

        void WriteTabs();

    private:
        CharBufferView m_writer;
        std::string m_dest;
        int32_t m_tabs : 31;
        int32_t m_tab_pending : 1;
        char m_new_line[4] = {};
    };

    template <typename TYPE>
        StringBuilder & operator << (StringBuilder & i_dest, Span<const TYPE> i_source)
    {
        for (size_t i = 0; i < i_source.size(); i++)
        {
            if(i != 0)
                i_dest << ", ";
            i_dest << i_source[i];
        }

        return i_dest;
    }

    template <typename... TYPE> std::string ToString(const TYPE &... i_objects)
    {
        StringBuilder builder;
        (builder << ... << i_objects);
        return builder.ShrinkAndGetString();
    }

} // namespace edi
