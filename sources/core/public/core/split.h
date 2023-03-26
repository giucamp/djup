
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <core/from_chars.h>
#include <iterator>
#include <stdexcept>

namespace core
{
    class Split
    {
    private:
        std::string_view m_source;
        char m_separator{};

    public:

        constexpr Split() noexcept = default;

        constexpr Split(const std::string_view & i_source, char i_separator) noexcept
            : m_source(i_source), m_separator(i_separator)
        {
        }

        struct end_marker_t{};

        static constexpr end_marker_t end_marker{};

        class const_iterator
        {
        public:
            using iterator_category = std::input_iterator_tag;

            constexpr const_iterator() noexcept = default;

            constexpr const_iterator(const std::string_view & i_source, char i_separator)
                : m_end_of_string(i_source.data() + i_source.size()), m_separator(i_separator)
            {
                m_token = get_next(i_source.data());
            }

            constexpr const std::string_view & operator*() const noexcept { return m_token; }

            constexpr const_iterator & operator++() noexcept
            {
                m_token = get_next(m_token.data() + m_token.size() + 1);
                return *this;
            }

            constexpr const_iterator operator++(int) noexcept
            {
                auto const copy{*this};
                m_token = get_next(m_token.data() + m_token.size() + 1);
                return copy;
            }

            constexpr bool operator == (end_marker_t) noexcept { return is_over(); }

            constexpr bool operator != (end_marker_t) noexcept { return !is_over(); }

        private:
            constexpr std::string_view get_next(const char * i_from) noexcept
            {
                auto curr = i_from;
                do
                {
                    if(curr >= m_end_of_string)
                    {
                        return {};
                    }
                } while(*++curr != m_separator);
                assert(curr >= i_from);
                return {i_from, static_cast<std::string_view::size_type>(curr - i_from)};
            }

            constexpr bool is_over() const noexcept { return m_token.data() == nullptr; }

        private:
            std::string_view  m_token;
            const char * m_end_of_string{};
            char m_separator{};
        };

        using iterator = const_iterator;

        constexpr const_iterator cbegin() const noexcept
        {
            return const_iterator(m_source, m_separator);
        }

        constexpr end_marker_t cend() const noexcept { return end_marker; }

        constexpr iterator begin() const noexcept { return iterator(m_source, m_separator); }

        constexpr end_marker_t end() const noexcept { return end_marker; }
    };

    constexpr Split::end_marker_t Split::end_marker;

} // namespace core
