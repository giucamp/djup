
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <string_view>
#include "djup/tensor.h"
#include "private/expression.h"

namespace djup
{
    std::shared_ptr<const Expression> ParseExpression(std::string_view i_source, 
        const std::shared_ptr<Scope> & i_parent_scope);

} // namespace djup
