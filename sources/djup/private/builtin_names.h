
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <private/name.h>

namespace djup
{
    namespace builtin_names
    {
        constexpr ConstexprName Add("Add");
        constexpr ConstexprName Mul("Mul");
        constexpr ConstexprName Pow("Pow");
        constexpr ConstexprName And("And");
        constexpr ConstexprName Or("Or");
        constexpr ConstexprName Not("Not");
        constexpr ConstexprName Equal("Equal");
        constexpr ConstexprName Less("Less");
        constexpr ConstexprName Stack("Stack");
        constexpr ConstexprName Is("Is");
        constexpr ConstexprName Scope("Scope");
        constexpr ConstexprName Tuple("Tuple");
    }
}
