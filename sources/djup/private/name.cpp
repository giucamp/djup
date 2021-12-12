
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

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
