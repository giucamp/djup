
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <string>
#include <string_view>
#include <functional> // for std::hash
#include <core/hash.h>
#include <core/to_chars.h>

namespace djup
{
    class ConstexprName
    {
    public:

        constexpr ConstexprName() = default;

        constexpr ConstexprName(std::string_view i_name)
            : m_name(i_name)
        {
            m_hash << m_name;
        }

        constexpr const std::string_view & AsString() const { return m_name; }

        constexpr Hash GetHash() const { return m_hash; }

        constexpr bool IsEmpty() const { return m_name.empty(); }

    private:
        std::string_view m_name;
        Hash m_hash;
    };

    class Name
    {
    public:

        Name() = default;

        Name(std::string i_name);

        Name(const char * i_name)
            : Name(std::string(i_name)) {}

        Name(std::string_view i_name)
            : Name(std::string(i_name)) { }

        Name(ConstexprName i_name);

        const std::string & AsString() const { return m_name; }

        Hash GetHash() const { return m_hash; }

        bool IsEmpty() const { return m_name.empty(); }

    private:
        std::string m_name;
        Hash m_hash;
    };

    bool operator == (const Name & i_first, const Name & i_second) noexcept;
    bool operator == (const ConstexprName & i_first, const Name & i_second) noexcept;
    bool operator == (const Name & i_first, const ConstexprName & i_second) noexcept;
    bool operator == (const ConstexprName & i_first, const ConstexprName & i_second) noexcept;

    bool operator != (const Name & i_first, const Name & i_second) noexcept;
    bool operator != (const ConstexprName & i_first, const Name & i_second) noexcept;
    bool operator != (const Name & i_first, const ConstexprName & i_second) noexcept;
    bool operator != (const ConstexprName & i_first, const ConstexprName & i_second) noexcept;

    template <> struct CharWriter<Name>
    {
        void operator() (CharBufferView & i_dest, const Name & i_source) noexcept
        {
            i_dest << i_source.AsString();
        }
    };

    template <> struct CharWriter<ConstexprName>
    {
        void operator() (CharBufferView & i_dest, const ConstexprName & i_source) noexcept
        {
            i_dest << i_source.AsString();
        }
    };

    inline Hash & operator << (Hash & i_dest, const Name & i_source)
    {
        return i_dest << i_source.GetHash();
    }

    inline Hash & operator << (Hash & i_dest, const ConstexprName & i_source)
    {
        return i_dest << i_source.GetHash();
    }
}

namespace std
{
    template <> struct hash<djup::Name>
    {
        size_t operator()(const djup::Name & i_source)
        {
            return std::hash<decltype(i_source.GetHash().GetValue())>{}(i_source.GetHash().GetValue());
        }
    };
}

