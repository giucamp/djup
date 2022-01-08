
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <djup/tensor.h>
#include <private/expression.h>
#include <private/parser.h>
#include <private/scope.h>
#include <private/builtin_names.h>
#include <core/algorithms.h>

namespace djup
{
    /*Tensor::Tensor(ScalarConst, double i_scalar)
        : m_expression(std::make_shared<Expression>(i_scalar))
    {

    }*/

    Tensor::Tensor(ScalarConst, int64_t i_scalar)
        : m_expression(MakeConstant(i_scalar).StealExpression())
    {

    }

    Tensor::Tensor(ScalarConst, bool i_scalar)
        : m_expression(MakeConstant(i_scalar).StealExpression())
    {

    }

    Tensor::Tensor(std::string_view i_expression)
        : Tensor(ParseExpression(i_expression, Scope::Root()))
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
        return i_dividend * Pow(i_divisor, MakeConstant<-1>());
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

    Tensor Is(const Tensor & i_tensor, const Tensor & i_pattern)
    {
        return MakeExpression(builtin_names::Is, {i_tensor, i_pattern});
    }

    Tensor MakeScope(Span<Tensor const> i_arguments)
    {
        return MakeExpression(builtin_names::Scope, i_arguments);
    }

    bool AlwaysEqual(const Tensor & i_first, const Tensor & i_second)
    {
        if(i_first.IsEmpty() || i_second.IsEmpty())
            return i_first.IsEmpty() == i_second.IsEmpty();

        return AlwaysEqual(*i_first.GetExpression(), *i_second.GetExpression());
    }

    Tensor Assign(const Tensor & i_left_hand_size, const Tensor & i_right_hand_side)
    {
        return MakeExpression(builtin_names::Assign, {i_left_hand_size, i_right_hand_side});
    }

    bool IsConstant(const Tensor & i_tensor)
    {
        return !i_tensor.IsEmpty() && i_tensor.GetExpression()->IsConstant();
    }

    bool IsVariable(const Tensor & i_tensor)
    {
        return !i_tensor.IsEmpty() && i_tensor.GetExpression()->IsVariable();
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

    std::string ToSimplifiedStringForm(const Tensor & i_source)
    {
        StringBuilder builder;
        ToSimplifiedStringForm(builder, i_source);
        return builder.ShrinkAndGetString();
    }
}
