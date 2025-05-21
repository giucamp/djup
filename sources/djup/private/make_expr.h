
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <private/common.h>
#include <djup/tensor.h>
#include <private/expression.h>

namespace djup
{
    [[nodiscard]] Tensor MakeExpression(
        const Namespace & i_namespace,
        TensorType i_tensor_type, Name i_name, 
        Span<const Tensor> i_arguments, 
        ExpressionMetadata i_metadata);

    [[nodiscard]] Tensor MakeLiteral(const Namespace & i_namespace, bool i_bool_value);

    [[nodiscard]] Tensor MakeLiteral(const Namespace & i_namespace, int64_t i_integer_value);

    [[nodiscard]] Tensor MakeReturn(const Namespace & i_namespace, Tensor i_value);

    [[nodiscard]] Tensor MakeNamespace(const Namespace & i_namespace, Span<Tensor const> i_statements);

    template <auto VALUE>
        [[nodiscard]] const Tensor & MakeLiteral(const Namespace & i_namespace)
    {
        if constexpr(std::is_same_v<decltype(VALUE), bool>)
        {
            static const Tensor s_value = MakeLiteral(i_namespace, VALUE);
            return s_value;
        }
        else
        {
            static_assert(std::is_integral_v<decltype(VALUE)>);
            static const Tensor s_value = MakeLiteral(i_namespace, NumericCast<int64_t>(VALUE));
            return s_value;
        }
    }

} // namespace djup

