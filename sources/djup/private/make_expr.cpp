
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <private/make_expr.h>
#include <private/builtin_names.h>

namespace djup
{
    Tensor MakeExpression(
        TensorType i_tensor_type, Name i_name,
        Span<const Tensor> i_arguments,
        ExpressionMetadata i_metadata)
    {
        return { std::make_shared<Expression>(std::move(i_tensor_type),
            std::move(i_name), i_arguments, std::move(i_metadata)) };
    }

    Tensor MakeLiteral(bool i_bool_value)
    {
        char buffer[8];
        Name name = ToCharsView(buffer, i_bool_value);

        ExpressionMetadata metadata;
        metadata.m_is_constant = true;
        metadata.m_is_literal = true;
        
        return MakeExpression(
            TensorType(builtin_names::Bool, ConstantShape{}),
            std::move(name), {}, std::move(metadata) );
    }

    Tensor MakeLiteral(int64_t i_integer_value)
    {
        char buffer[std::numeric_limits<int64_t>::max_digits10 + 4];
        Name name = ToCharsView(buffer, i_integer_value);

        ExpressionMetadata metadata;
        metadata.m_is_constant = true;
        metadata.m_is_literal = true;

        return MakeExpression(
            TensorType(builtin_names::Int, ConstantShape{}),
            std::move(name), {}, std::move(metadata));
    }

    Tensor MakeReturn(Tensor i_value)
    {
        return MakeExpression({}, builtin_names::Return, { i_value }, {});
    }

    Tensor MakeNamespace(Span<Tensor const> i_statements)
    {
        return MakeExpression({}, builtin_names::Namespace, i_statements, {});
    }

} // namespace djup
