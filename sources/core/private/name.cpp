
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <core/name.h>
#include <string.h>

namespace core
{
    Name::Name() = default;

    Name::Name(const std::string & i_name)
        : m_string(i_name.begin(), i_name.end())
    {
        m_hash << m_string;
    }

    Name::Name(const char* i_name)
        : m_string(i_name, i_name + strlen(i_name))
    {
        m_hash << m_string;
    }

    Name::Name(std::string_view i_name)
        : m_string(i_name.begin(), i_name.end())
    {
        m_hash << m_string;
    }

    Name::Name(ConstexprName i_name)
        : m_string(i_name.AsString().begin(), i_name.AsString().end()),
          m_hash(i_name.GetHash())
    {

    }

    bool operator == (const Name& i_first, const Name& i_second) noexcept
    {
        return i_first.GetHash() == i_second.GetHash() && i_first.AsString() == i_second.AsString();
    }

    bool operator == (const ConstexprName& i_first, const Name& i_second) noexcept
    {
        return i_first.GetHash() == i_second.GetHash() && i_first.AsString() == i_second.AsString();
    }

    bool operator == (const Name& i_first, const ConstexprName& i_second) noexcept
    {
        return i_first.GetHash() == i_second.GetHash() && i_first.AsString() == i_second.AsString();
    }

    bool operator == (const ConstexprName& i_first, const ConstexprName& i_second) noexcept
    {
        return i_first.GetHash() == i_second.GetHash() && i_first.AsString() == i_second.AsString();
    }

    bool operator != (const Name& i_first, const Name& i_second) noexcept
    {
        return !(i_first == i_second);
    }

    bool operator != (const ConstexprName& i_first, const Name& i_second) noexcept
    {
        return !(i_first == i_second);
    }

    bool operator != (const Name& i_first, const ConstexprName& i_second) noexcept
    {
        return !(i_first == i_second);
    }

    bool operator != (const ConstexprName& i_first, const ConstexprName& i_second) noexcept
    {
        return !(i_first == i_second);
    }
}
