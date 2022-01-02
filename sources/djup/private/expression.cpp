
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
#include <algorithm>

namespace djup
{
    Expression::Expression(bool i_bool_literal)
        : m_type(Domain::Bool, ConstantShape{})
    {
        char buffer[8];
        m_name = ToCharsView(buffer, i_bool_literal);

        m_hash << m_name;
        m_hash << m_type;

        m_is_constant = true;
        m_is_bool_literal = true;
    }

    Expression::Expression(int64_t i_integer_literal)
        : m_type(Domain::Integer, ConstantShape{})
    {
        char buffer[std::numeric_limits<int64_t>::max_digits10 + 4];
        m_name = ToCharsView(buffer, i_integer_literal);

        m_hash << m_name;
        m_hash << m_type;

        m_is_constant = true;
        m_is_integer_literal = true;
    }

    Expression::Expression(Name i_name, TensorType i_type, Span<const Tensor> i_arguments)
        : m_name(std::move(i_name)), m_type(std::move(i_type)),
          m_arguments(i_arguments.begin(), i_arguments.end())
    {
        m_hash << m_name;
        m_hash << m_type;
        m_hash << m_arguments;

        m_is_constant = std::all_of(m_arguments.begin(), m_arguments.end(), 
            [](const Tensor i_arg){ return i_arg.GetExpression()->IsConstant(); });
    }

    Hash & operator << (Hash & i_dest, const Tensor & i_src)
    {
        if(!i_src.IsEmpty())
            return i_dest << i_src.GetExpression()->GetHash();
        else
            i_dest << 1;
        return i_dest;
    }

    Tensor MakeLiteralExpression(bool i_bool_value)
    {
        return {std::make_shared<Expression>(i_bool_value)};
    }

    Tensor MakeLiteralExpression(int64_t i_integer_value)
    {
        return {std::make_shared<Expression>(i_integer_value)};
    }

    Tensor MakeExpression(Name i_name, Span<const Tensor> i_arguments)
    {
        return MakeExpression(std::move(i_name), TensorType{Domain::Any, std::monostate{}}, i_arguments);
    }

    Tensor MakeExpression(Name i_name, TensorType i_type, Span<const Tensor> i_arguments)
    {
        return {std::make_shared<Expression>(std::move(i_name), std::move(i_type), i_arguments)};
    }

    bool IsConstant(const Tensor & i_tensor)
    {
        return i_tensor.GetExpression()->IsConstant();
    }

    bool IsVariable(const Tensor & i_tensor)
    {
        return i_tensor.GetExpression()->GetArguments().empty();
    }

    bool IsType(const Tensor & i_tensor)
    {
        const Name & name = i_tensor.GetExpression()->GetName();

        if(name.IsEmpty())
            return true;
        else if(name == builtin_names::Tuple)
            return AllOf(i_tensor.GetExpression()->GetArguments(), IsType );

        return false;
    }

    bool AlwaysEqual(const Expression & i_first, const Expression & i_second)
    {
        if(i_first.GetHash() != i_second.GetHash())
            return false;

        if(i_first.GetName() != i_second.GetName())
            return false;

        if(i_first.GetType() != i_second.GetType())
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
