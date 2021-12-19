
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <djup/tensor.h>
#include <private/expression.h>
#include <private/parser.h>
#include <private/scope.h>

namespace djup
{
    /*Tensor::Tensor(ScalarConst, double i_scalar)
        : m_expression(std::make_shared<Expression>(i_scalar))
    {

    }*/

    Tensor::Tensor(ScalarConst, int64_t i_scalar)
        : m_expression(std::make_shared<Expression>(Expression::IntegerConstant{i_scalar}))
    {

    }

    Tensor::Tensor(ScalarConst, bool i_scalar)
        : m_expression(std::make_shared<Expression>(Expression::BoolConstant{i_scalar}))
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

    void CharWriter<Tensor>::operator() (CharBufferView & i_dest, const Tensor & i_source)
    {
        Error("Not implemented");
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
        static const Name name("Add");
        return MakeTensorExpression(name, i_arguments);
    }

    Tensor Mul(Span<Tensor const> i_arguments)
    {
        static const Name name("Mul");
        return MakeTensorExpression(name, i_arguments);
    }

    Tensor Pow(Tensor const & i_base, Tensor const & i_exp)
    {
        static const Name name("Pow");
        return MakeTensorExpression(name, {i_base, i_exp});
    }

    Tensor And(Span<Tensor const> i_arguments)
    {
        static const Name name("And");
        return MakeTensorExpression(name, i_arguments);
    }

    Tensor Or(Span<Tensor const> i_arguments)
    {
        static const Name name("Or");
        return MakeTensorExpression(name, i_arguments);
    }

    Tensor Not(const Tensor & i_argument)
    {
        static const Name name("Not");
        return MakeTensorExpression(name, {i_argument});
    }

    Tensor Equal(const Tensor & i_first, const Tensor & i_second)
    {
        static const Name name("Equal");
        return MakeTensorExpression(name, {i_first, i_second});
    }

    Tensor Less(const Tensor & i_first, const Tensor & i_second)
    {
        static const Name name("Less");
        return MakeTensorExpression(name, {i_first, i_second});
    }

    Tensor Stack(Span<Tensor const> i_arguments)
    {
        static const Name name("Stack");
        return MakeTensorExpression(name, i_arguments);
    }
}
