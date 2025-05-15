
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <private/common.h>
#include <string_view>
#include "djup/tensor.h"
#include <private/expression.h>

namespace djup
{
    Tensor ParseExpression(std::string_view i_source);

    std::shared_ptr<Namespace> ParseNamespace(std::string_view i_source);

} // namespace djup
