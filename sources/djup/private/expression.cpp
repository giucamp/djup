
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/expression.h>
#include <private/scope.h>
#include <private/type.h>
#include <private/builtin_names.h>
#include <core/algorithms.h>
#include <core/to_chars.h>
#include <djup/tensor.h>
#include <core/algorithms.h>

namespace djup
{
    Expression::Expression(ExpressionData && i_data)
    {
        m_data = std::move(i_data);

        if(m_data.m_is_bool_literal || m_data.m_is_integer_literal)
        {
            assert(m_data.m_is_constant); 
        }

        assert(!(m_data.m_is_constant && m_data.m_is_variable));

        if(!m_data.m_is_variable && AllOf(m_data.m_arguments, djup::IsConstant))
            m_data.m_is_constant = true;

        m_hash << m_data.m_name;
        m_hash << m_data.m_type;
        m_hash << m_data.m_arguments;
        m_hash << m_data.m_all_flags;
    }

    Hash & operator << (Hash & i_dest, const Tensor & i_src)
    {
        if(!i_src.IsEmpty())
            return i_dest << i_src.GetExpression()->GetHash();
        else
            i_dest << 1;
        return i_dest;
    }

    Tensor MakeConstant(bool i_bool_value)
    {
        ExpressionData data;
        char buffer[8];
        data.m_name = ToCharsView(buffer, i_bool_value);
        data.m_type = {Domain::Bool, ConstantShape{}};
        data.m_is_constant = true;
        data.m_is_bool_literal = true;

        return {std::make_shared<Expression>(std::move(data))};
    }

    Tensor MakeConstant(int64_t i_integer_value)
    {
        ExpressionData data;
        char buffer[std::numeric_limits<int64_t>::max_digits10 + 4];
        data.m_name = ToCharsView(buffer, i_integer_value);
        data.m_type = {Domain::Integer, ConstantShape{}};
        data.m_is_constant = true;
        data.m_is_integer_literal = true;

        return {std::make_shared<Expression>(std::move(data))};
    }

    Tensor MakeVariable(Name i_name, TensorType i_type)
    {
        ExpressionData data;
        data.m_name = std::move(i_name);
        data.m_type = std::move(i_type);
        data.m_is_variable = true;
        return {std::make_shared<Expression>(std::move(data))};
    }

    Tensor MakeExpression(ExpressionData && i_data)
    {
        return {std::make_shared<Expression>(std::move(i_data))};
    }

    Tensor MakeExpression(Name i_name, Span<const Tensor> i_arguments)
    {
        return MakeExpression(std::move(i_name), TensorType{Domain::Any, std::monostate{}}, i_arguments);
    }

    Tensor MakeExpression(Name i_name, TensorType i_type, Span<const Tensor> i_arguments)
    {
        ExpressionData data;
        data.m_name = std::move(i_name);
        data.m_type = std::move(i_type);
        data.m_arguments = {i_arguments.begin(), i_arguments.end()};
        return {std::make_shared<Expression>(std::move(data))};
    }

    bool AlwaysEqual(const Expression & i_first, const Expression & i_second)
    {
        if(i_first.GetHash() != i_second.GetHash())
            return false;

        if(i_first.GetName() != i_second.GetName())
            return false;

        if(i_first.GetType() != i_second.GetType())
            return false;

        if(i_first.GetAllFlags() != i_second.GetAllFlags())
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
