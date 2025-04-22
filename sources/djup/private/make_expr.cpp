
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <private/make_expr.h>
#include <private/builtin_names.h>

namespace djup
{
    Tensor MakeExpression(Name i_name, Span<const Tensor> i_arguments,
        std::optional<ExpressionMetadata> i_metadata)
    {
        return { std::make_shared<Expression>(std::move(i_name), i_arguments, std::move(i_metadata)) };
    }

    Tensor MakeLiteral(bool i_bool_value)
    {
        char buffer[8];
        Name name = ToCharsView(buffer, i_bool_value);
        ExpressionMetadata metadata = { MakeTensorType(builtin_names::Bool, {}) };
        metadata.m_is_constant = true;
        return MakeExpression(builtin_names::Literal, { MakeExpression(std::move(name), {}) }, std::move(metadata));
    }

    Tensor MakeLiteral(int64_t i_integer_value)
    {
        char buffer[std::numeric_limits<int64_t>::max_digits10 + 4];
        Name name = ToCharsView(buffer, i_integer_value);
        ExpressionMetadata metadata = { MakeTensorType(builtin_names::Int, {}) };
        metadata.m_is_constant = true;
        return MakeExpression(builtin_names::Literal, { MakeExpression(std::move(name), {}) }, std::move(metadata));
    }

    Tensor MakeTensorType(Name i_scalar_type, Tensor i_shape_vector)
    {
        Tensor res = MakeExpression(builtin_names::TensorType,
            { MakeExpression(i_scalar_type), std::move(i_shape_vector) });
        return res;
    }

    Tensor MakeTensorType(Tensor i_scalar_type, Tensor i_shape)
    {
        return MakeExpression(builtin_names::TensorType, { i_scalar_type, i_shape });
    }

    Tensor MakeIdentifier(Tensor i_type, Tensor i_name, Span<const Tensor> i_arguments)
    {
        std::vector<Tensor> arguments;
        arguments.reserve(i_arguments.size() + 2);
        arguments.push_back(std::move(i_type));
        arguments.push_back(std::move(i_name));
        for (const Tensor& argument : i_arguments)
            arguments.push_back(argument);
        return MakeExpression(builtin_names::Identifier, arguments);
    }

} // namespace djup
