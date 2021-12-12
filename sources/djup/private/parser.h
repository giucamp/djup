
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <string_view>
#include "djup/tensor.h"
#include <private/expression.h>

namespace djup
{
    Tensor ParseExpression(std::string_view i_source,
        const std::shared_ptr<const Scope> & i_parent_scope);

} // namespace djup
