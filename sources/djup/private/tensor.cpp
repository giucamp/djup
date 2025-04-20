
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <djup/tensor.h>
#include <djup/expression.h>
#include <private/parser.h>
#include <private/namespace.h>
#include <private/builtin_names.h>
#include <core/algorithms.h>

namespace djup
{
    namespace
    {
        const std::shared_ptr<const Expression> & GetEmptyExpression()
        {
            static std::shared_ptr<const Expression> expr = std::make_shared<Expression>();
            return expr;
        }
    }

    Tensor::Tensor()
        : m_expression(GetEmptyExpression())
    {

    }

    Tensor::Tensor(Tensor && i_source) noexcept
        : m_expression(std::move(i_source.m_expression))
    {
        i_source.m_expression = GetEmptyExpression();
    }

    Tensor & Tensor::operator = (Tensor && i_source) noexcept
    {
        m_expression = std::move(i_source.m_expression);
        i_source.m_expression = GetEmptyExpression();
        return *this;
    }

    Tensor::Tensor(const Tensor &) = default;
    
    Tensor & Tensor::operator = (const Tensor &) = default;

    const Tensor & EmptyTensor()
    {
        static const Tensor s_empty;
        return s_empty;
    }

    bool IsEmpty(const Tensor & i_tensor)
    {
        return i_tensor.GetExpression() == GetEmptyExpression();
    }

    Tensor::Tensor(ScalarConst, int64_t i_scalar)
        : m_expression(MakeLiteral(i_scalar).StealExpression())
    {

    }

    Tensor::Tensor(ScalarConst, bool i_scalar)
        : m_expression(MakeLiteral(i_scalar).StealExpression())
    {

    }

    Tensor::Tensor(std::string_view i_expression)
        : Tensor(ParseExpression(i_expression))
    {

    }

    const std::shared_ptr<const Expression> & Tensor::GetExpression() const
    { 
        if(m_expression)
            return m_expression; 
        else
            Error("Trying to retrieve the expression from an empty tensor");
    }

    std::shared_ptr<const Expression> Tensor::StealExpression()
    {
        if(m_expression)
            return std::move(m_expression);
        else
            Error("Trying to steal the expression from an empty tensor");
    }

    Tensor TensorType(Tensor i_scalar_type, Tensor i_shape)
    {
        return MakeExpression(builtin_names::TensorType, {i_scalar_type, i_shape});
    }

    Tensor Identifier(Tensor i_type, Tensor i_name, Span<const Tensor> i_arguments)
    {
        std::vector<Tensor> arguments;
        arguments.reserve(i_arguments.size() + 2);
        arguments.push_back(std::move(i_type));
        arguments.push_back(std::move(i_name));
        for(const Tensor & argument : i_arguments)
            arguments.push_back(argument);
        return MakeExpression(builtin_names::Identifier, arguments);
    }

    const Name & GetIdentifierName(const Tensor & i_identifier)
    {
        if(i_identifier.GetExpression()->GetName() != builtin_names::Identifier)
            Error("GetIdentifierName - not a identifier");

        const Tensor & name_arg = i_identifier.GetExpression()->GetArguments().at(1);
        return name_arg.GetExpression()->GetName();
    }

    Tensor operator + (const Tensor & i_operand)
    {
        return i_operand;
    }

    Tensor operator - (const Tensor & i_operand)
    {
        return i_operand * -1;
    }

    Tensor operator + (const Tensor & i_first, const Tensor & i_second)
    {
        return Add({ i_first, i_second });
    }

    Tensor operator - (const Tensor & i_first, const Tensor & i_second)
    {
        return Add({ i_first, -i_second });
    }

    Tensor & operator += (Tensor & i_first, const Tensor & i_second)
    {
        return i_first = i_first + i_second;
    }

    Tensor & operator -= (Tensor & i_first, const Tensor & i_second)
    {
        return i_first = i_first - i_second;
    }

    Tensor operator * (const Tensor & i_first, const Tensor& i_second)
    {
        return Mul({ i_first, i_second });
    }

    Tensor operator / (const Tensor & i_dividend, const Tensor & i_divisor)
    {
        return i_dividend * Pow(i_divisor, MakeLiteral<-1>());
    }

    Tensor & operator *= (Tensor & i_first, const Tensor & i_second)
    {
        return i_first = i_first * i_second;
    }

    Tensor & operator /= (Tensor & i_dividend, const Tensor & i_divisor)
    {
        return i_dividend = i_dividend / i_divisor;
    }

    Tensor operator && (const Tensor & i_first_bool, const Tensor & i_second_bool)
    {
        return And({ i_first_bool, i_second_bool });
    }

    Tensor operator || (const Tensor & i_first_bool, const Tensor & i_second_bool)
    {
        return Or({ i_first_bool, i_second_bool });
    }

    Tensor operator ! (const Tensor & i_bool_operand)
    {
        return Not(i_bool_operand);
    }

    Tensor operator == (const Tensor & i_first, const Tensor & i_second)
    {
        return Equal(i_first, i_second);
    }

    Tensor operator != (const Tensor & i_first, const Tensor & i_second)
    {
        return !Equal(i_first, i_second);
    }

    Tensor operator < (const Tensor & i_first, const Tensor & i_second)
    {
        return Less(i_first, i_second);
    }

    Tensor operator >= (const Tensor& i_first, const Tensor& i_second)
    {
        return !Less(i_first, i_second);
    }

    Tensor operator <= (const Tensor& i_first, const Tensor& i_second)
    {
        return i_first < i_second || i_first == i_second;
    }

    Tensor operator > (const Tensor & i_first, const Tensor & i_second)
    {
        return !(i_first <= i_second);
    } 

    Tensor Add(Span<Tensor const> i_arguments)
    {
        return MakeExpression(builtin_names::Add, i_arguments);
    }

    Tensor Mul(Span<Tensor const> i_arguments)
    {
        return MakeExpression(builtin_names::Mul, i_arguments);
    }

    Tensor Pow(Tensor const & i_base, Tensor const & i_exp)
    {
        return MakeExpression(builtin_names::Pow, {i_base, i_exp});
    }

    Tensor And(Span<Tensor const> i_arguments)
    {
        return MakeExpression(builtin_names::And, i_arguments);
    }

    Tensor Or(Span<Tensor const> i_arguments)
    {
        return MakeExpression(builtin_names::Or, i_arguments);
    }

    Tensor Not(const Tensor & i_argument)
    {
        return MakeExpression(builtin_names::Not, {i_argument});
    }

    Tensor Equal(const Tensor & i_first, const Tensor & i_second)
    {
        return MakeExpression(builtin_names::Equal, {i_first, i_second});
    }

    Tensor Less(const Tensor & i_first, const Tensor & i_second)
    {
        return MakeExpression(builtin_names::Less, {i_first, i_second});
    }

    Tensor Stack(Span<Tensor const> i_arguments)
    {
        return MakeExpression(builtin_names::Stack, i_arguments);
    }

    Tensor Tuple(Span<Tensor const> i_arguments)
    {
        return MakeExpression(builtin_names::Tuple, i_arguments);
    }

    Tensor If(Span<Tensor const> i_operands)
    {
        return MakeExpression(builtin_names::If, i_operands);
    }

    Tensor MakeNamespace(Span<Tensor const> i_arguments)
    {
        return MakeExpression(builtin_names::Namespace, i_arguments);
    }

    bool AlwaysEqual(const Tensor & i_first, const Tensor & i_second)
    {
        return AlwaysEqual(*i_first.GetExpression(), *i_second.GetExpression());
    }

    bool IsConstant(const Tensor & i_tensor)
    {
        return i_tensor.GetExpression()->IsConstant();
    }

    bool IsIdentifier(const Tensor & i_tensor)
    {
        return NameIs(i_tensor, builtin_names::Identifier);
    }

    bool IsLiteral(const Tensor& i_tensor)
    {
        return NameIs(i_tensor, builtin_names::Literal);
    }

    bool IsTensorType(const Tensor & i_tensor)
    {
        return NameIs(i_tensor, builtin_names::TensorType);
    }

    Tensor RepetitionsZeroToMany(Span<Tensor const> i_tensors)
    {
        return MakeExpression(builtin_names::RepetitionsZeroToMany, i_tensors);
    }

    Tensor RepetitionsOneToMany(Span<Tensor const> i_tensors)
    {
        return MakeExpression(builtin_names::RepetitionsOneToMany, i_tensors);
    }

    Tensor RepetitionsZeroToOne(Span<Tensor const> i_tensors)
    {
        return MakeExpression(builtin_names::RepetitionsZeroToOne, i_tensors);
    }

    std::string ToSimplifiedStringForm(const Tensor & i_source, size_t i_depth, bool tidy)
    {
        StringBuilder builder;
        ToSimplifiedStringForm(builder, i_source, i_depth, tidy);
        return builder.ShrinkAndGetString();
    }
}
