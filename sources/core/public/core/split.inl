
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace core
{
    constexpr Split::Split() noexcept = default;

    constexpr Split::Split(const std::string_view & i_source, char i_separator) noexcept
        : m_source(i_source), m_separator(i_separator)
    {
    }

    constexpr Split::const_iterator::const_iterator() noexcept = default;

    constexpr Split::const_iterator::const_iterator(const std::string_view & i_source, char i_separator)
        : m_end_of_string(i_source.data() + i_source.size()), m_separator(i_separator)
    {
        m_token = get_next(i_source.data());
    }

    constexpr const std::string_view & Split::const_iterator::operator *() const noexcept
    { 
        return m_token; 
    }

    constexpr Split::const_iterator & Split::const_iterator::operator++() noexcept
    {
        m_token = get_next(m_token.data() + m_token.size() + 1);
        return *this;
    }

    constexpr Split::const_iterator Split::const_iterator::operator ++ (int) noexcept
    {
        auto const copy{ *this };
        m_token = get_next(m_token.data() + m_token.size() + 1);
        return copy;
    }

    constexpr bool Split::const_iterator::operator == (end_marker_t) noexcept
    {
        return is_over();
    }

    constexpr bool Split::const_iterator::operator != (end_marker_t) noexcept
    { 
        return !is_over(); 
    }

    constexpr std::string_view Split::const_iterator::get_next(const char * i_from) noexcept
    {
        auto curr = i_from;
        do
        {
            if (curr >= m_end_of_string)
            {
                return {};
            }
        } while (*++curr != m_separator);
        assert(curr >= i_from);
        return { i_from, static_cast<std::string_view::size_type>(curr - i_from) };
    }

    constexpr bool Split::const_iterator::is_over() const noexcept
    { 
        return m_token.data() == nullptr;
    }

    constexpr Split::const_iterator Split::cbegin() const noexcept
    {
        return const_iterator(m_source, m_separator);
    }

    constexpr Split::end_marker_t Split::cend() const noexcept { return end_marker; }

    constexpr Split::iterator Split::begin() const noexcept { return iterator(m_source, m_separator); }

    constexpr Split::end_marker_t Split::end() const noexcept { return end_marker; }

    constexpr Split::end_marker_t Split::end_marker;

} // namespace core
