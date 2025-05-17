
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <core/from_chars.h>
#include <iterator>
#include <stdexcept>

namespace core
{
    /** Splits a string in tokens, using the specified separator char.
        Tokens are returned with std::string_view's pointing to the
        source buffer (no dynamic or fixed-size string buffers).
        Example:
        
        for(std::string_view word : Split("abc def fgh", ' '))
        {
            ...
        }
    */
    class Split
    {
    private:
        std::string_view m_source;
        char m_separator{};

    public:

        constexpr Split() noexcept;

        constexpr Split(const std::string_view & i_source, char i_separator) noexcept;

        struct end_marker_t{};

        static constexpr end_marker_t end_marker{};

        class const_iterator
        {
        public:
            using iterator_category = std::forward_iterator_tag;

            constexpr const_iterator() noexcept;

            constexpr const_iterator(const std::string_view & i_source, char i_separator);

            constexpr const std::string_view & operator * () const noexcept;

            constexpr const_iterator & operator ++ () noexcept;

            constexpr const_iterator operator ++ (int) noexcept;

            constexpr bool operator == (end_marker_t) noexcept;

            constexpr bool operator != (end_marker_t) noexcept;

        private:
            constexpr std::string_view get_next(const char * i_from) noexcept;

            constexpr bool is_over() const noexcept;

        private:
            std::string_view  m_token;
            const char * m_end_of_string{};
            char m_separator{};
        };

        using iterator = const_iterator;

        constexpr const_iterator cbegin() const noexcept;

        constexpr end_marker_t cend() const noexcept;

        constexpr iterator begin() const noexcept;

        constexpr end_marker_t end() const noexcept;
    };

} // namespace core

#include <core/split.inl>
