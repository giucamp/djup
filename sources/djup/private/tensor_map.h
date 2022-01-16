
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <djup/tensor.h>
#include <private/expression.h>
#include <unordered_map>

namespace djup
{
    namespace detail
    {
        struct StdTensorHash
        {
            size_t operator() (const Tensor & i_source) const
            {
                return i_source.GetExpression()->GetHash().ToSizeT();
            }
        };

        struct StdTensorEqual
        {
            size_t operator() (const Tensor & i_first, const Tensor & i_second) const
            {
                return i_first.GetExpression() == i_second.GetExpression();
            }
        };
    }

    template <typename VALUE>
        using TensorMap = std::unordered_map<Tensor, VALUE, detail::StdTensorHash, detail::StdTensorEqual>;
}
