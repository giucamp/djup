
#pragma once
#include <private/name.h>

namespace djup
{
    Name::Name(std::string i_name)
        : m_name(std::move(i_name))
    {
        m_hash = ComputeHash(m_name);
    }
}
