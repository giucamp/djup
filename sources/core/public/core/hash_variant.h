
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <core/hash.h>
#include <variant>

namespace djup
{
    template <typename... TYPES>
        Hash & operator << (Hash & i_dest, const std::variant<TYPES...> & i_source)
    {
        i_dest << static_cast<int64_t>(i_source.index());
        std::visit([&i_dest](auto && i_alternative){
            i_dest << i_alternative;
        }, i_source);
        return i_dest;
    }

    inline Hash & operator << (Hash & i_dest, std::monostate)
    {
        return i_dest;
    }
}
