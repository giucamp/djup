
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <core/immutable_vector.h>
#include <core/hash.h>
#include <core/to_chars.h>
#include <string>
#include <string_view>
#include <functional> // for std::hash

namespace core
{

    class ConstexprName
    {
    public:

        constexpr ConstexprName() = default;

        constexpr ConstexprName(std::string_view i_name)
            : m_string(i_name)
        {
            m_hash << m_string;
        }

        constexpr const std::string_view& AsString() const { return m_string; }

        constexpr Hash GetHash() const { return m_hash; }

        constexpr bool IsEmpty() const { return m_string.empty(); }

    private:
        std::string_view m_string;
        Hash m_hash;
    };

    class Name
    {
    public:

        Name() = default;

        Name(const std::string & i_name);

        Name(const char* i_name);

        Name(std::string_view i_name);

        Name(ConstexprName i_name);

        std::string AsString() const { return { m_string.data(), m_string.size() }; }

        std::string_view AsStringView() const { return { m_string.data(), m_string.size() }; }

        Hash GetHash() const { return m_hash; }

        bool IsEmpty() const { return m_string.empty(); }

    private:
        ImmutableVector<char> m_string;
        Hash m_hash;
    };

    bool operator == (const Name& i_first, const Name& i_second) noexcept;
    bool operator == (const ConstexprName& i_first, const Name& i_second) noexcept;
    bool operator == (const Name& i_first, const ConstexprName& i_second) noexcept;
    bool operator == (const ConstexprName& i_first, const ConstexprName& i_second) noexcept;

    bool operator != (const Name& i_first, const Name& i_second) noexcept;
    bool operator != (const ConstexprName& i_first, const Name& i_second) noexcept;
    bool operator != (const Name& i_first, const ConstexprName& i_second) noexcept;
    bool operator != (const ConstexprName& i_first, const ConstexprName& i_second) noexcept;

    inline Hash& operator << (Hash& i_dest, const Name& i_source)
    {
        return i_dest << i_source.GetHash();
    }

    inline Hash& operator << (Hash& i_dest, const ConstexprName& i_source)
    {
        return i_dest << i_source.GetHash();
    }

} // namespace core

namespace core
{
    template <> struct CharWriter<core::Name>
    {
        void operator() (CharBufferView& i_dest, const core::Name& i_source) noexcept
        {
            i_dest << i_source.AsString();
        }
    };

    template <> struct CharWriter<core::ConstexprName>
    {
        void operator() (CharBufferView& i_dest, const core::ConstexprName& i_source) noexcept
        {
            i_dest << i_source.AsString();
        }
    };


} // namespace core

namespace std
{
    template <> struct hash<core::Name>
    {
        size_t operator()(const core::Name& i_source) const
        {
            return std::hash<decltype(i_source.GetHash().GetValue())>{}(i_source.GetHash().GetValue());
        }
    };

} // namespace std
