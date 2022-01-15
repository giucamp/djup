
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/expression.h>
#include <private/namespace.h>
#include <private/builtin_names.h>
#include <core/algorithms.h>
#include <core/to_chars.h>
#include <djup/tensor.h>
#include <core/algorithms.h>

namespace djup
{
    Expression::Expression(ExpressionData && i_data)
        : m_data(std::move(i_data))
    {
        if(m_data.m_name != builtin_names::Identifier 
                && AllOf(m_data.m_arguments, djup::IsConstant))
            m_data.m_is_constant = true;

        m_hash << m_data.m_name;
        m_hash << m_data.m_arguments;
    }

    Tensor Expression::GetType() const
    { 
        if(m_data.m_type)
            return m_data.m_type; 
        else
            return {};
    }

    Hash & operator << (Hash & i_dest, const Tensor & i_src)
    {
        return i_dest << i_src.GetExpression()->GetHash();
    }

    Tensor MakeExpression(ExpressionData && i_data)
    {
        return {std::make_shared<Expression>(std::move(i_data))};
    }

    Tensor MakeExpression(Name i_name, Tensor i_type, Span<const Tensor> i_arguments)
    {
        ExpressionData data;
        data.m_name = std::move(i_name);
        data.m_type = i_type.StealExpression();
        data.m_arguments = {i_arguments.begin(), i_arguments.end()};
        return {std::make_shared<Expression>(std::move(data))};
    }

    Tensor MakeConstantExpression(Name i_name, Tensor i_type, Span<const Tensor> i_arguments)
    {
        ExpressionData data;
        data.m_name = std::move(i_name);
        data.m_type = i_type.StealExpression();
        data.m_is_constant = true;
        data.m_arguments = {i_arguments.begin(), i_arguments.end()};
        return {std::make_shared<Expression>(std::move(data))};
    }

    Tensor MakeLiteral(bool i_bool_value)
    {
        ExpressionData data;
        char buffer[8];
        Name name = ToCharsView(buffer, i_bool_value);

        Tensor type = TensorType(builtin_names::Bool, {});
        return MakeConstantExpression(builtin_names::Literal, type, { MakeConstantExpression(std::move(name), {}) });
    }

    Tensor MakeLiteral(int64_t i_integer_value)
    {
        ExpressionData data;
        char buffer[std::numeric_limits<int64_t>::max_digits10 + 4];
        Name name = ToCharsView(buffer, i_integer_value);

        Tensor type = TensorType(builtin_names::Int, {});
        return MakeConstantExpression(builtin_names::Literal, type, { MakeConstantExpression(std::move(name), {}) });
    }

    Tensor TensorType(Name i_scalar_type, Tensor i_shape_vector)
    {
        return MakeConstantExpression(builtin_names::TensorType, {},
            { MakeConstantExpression(i_scalar_type, {}), std::move(i_shape_vector) });
    }

    bool NameIs(const Tensor & i_tensor, const Name & i_name)
    {
        return i_tensor.GetExpression()->GetName() == i_name;
    }

    bool NameIs(const Tensor & i_tensor, const ConstexprName & i_name)
    {
        return i_tensor.GetExpression()->GetName() == i_name;
    }

    bool AlwaysEqual(const Expression & i_first, const Expression & i_second)
    {
        if(i_first.GetHash() != i_second.GetHash())
            return false;

        if(i_first.GetName() != i_second.GetName())
            return false;

        const size_t argument_count = i_first.GetArguments().size();
        if(argument_count != i_second.GetArguments().size())
            return false;

        for(size_t argument_index = 0; argument_index < argument_count; argument_index++)
            if(!AlwaysEqual(i_first.GetArgument(argument_index), i_second.GetArgument(argument_index)))
                return false;

        return true;
    }
}
