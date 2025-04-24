
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <private/common.h>
#include <djup/tensor.h>
#include <djup/expression.h>

namespace djup
{
    [[nodiscard]] Tensor MakeExpression(
        TensorType i_tensor_type, Name i_name, 
        Span<const Tensor> i_arguments, 
        ExpressionMetadata i_metadata);

    [[nodiscard]] Tensor MakeLiteral(bool i_bool_value);

    [[nodiscard]] Tensor MakeLiteral(int64_t i_integer_value);

    [[nodiscard]] Tensor MakeReturn(Tensor i_value);

    [[nodiscard]] Tensor MakeNamespace(Span<Tensor const> i_statements);

    template <auto VALUE>
        [[nodiscard]] const Tensor & MakeLiteral()
    {
        if constexpr(std::is_same_v<decltype(VALUE), bool>)
        {
            static const Tensor s_value = MakeLiteral(VALUE);
            return s_value;
        }
        else
        {
            static_assert(std::is_integral_v<decltype(VALUE)>);
            static const Tensor s_value = MakeLiteral(NumericCast<int64_t>(VALUE));
            return s_value;
        }
    }

} // namespace djup

