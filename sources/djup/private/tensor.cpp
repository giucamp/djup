
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
    namespace
    {
        const std::shared_ptr<const Expression> & GetEmptyExpression()
        {
            static std::shared_ptr<const Expression> expr = std::make_shared<Expression>(ExpressionData{});
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
        return MakeExpression(builtin_names::TensorType, {}, {i_scalar_type, i_shape});
    }

    Tensor Identifier(Tensor i_type, Tensor i_name, Span<const Tensor> i_arguments)
    {
        std::vector<Tensor> arguments;
        arguments.reserve(i_arguments.size() + 2);
        arguments.push_back(i_type);
        arguments.push_back(std::move(i_name));
        for(const Tensor & argument : i_arguments)
            arguments.push_back(argument);
        return MakeExpression(builtin_names::Identifier, std::move(i_type), arguments);
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
        return MakeExpression(builtin_names::Add, {}, i_arguments);
    }

    Tensor Mul(Span<Tensor const> i_arguments)
    {
        return MakeExpression(builtin_names::Mul, {}, i_arguments);
    }

    Tensor Pow(Tensor const & i_base, Tensor const & i_exp)
    {
        return MakeExpression(builtin_names::Pow, {}, {i_base, i_exp});
    }

    Tensor And(Span<Tensor const> i_arguments)
    {
        return MakeExpression(builtin_names::And, {}, i_arguments);
    }

    Tensor Or(Span<Tensor const> i_arguments)
    {
        return MakeExpression(builtin_names::Or, {}, i_arguments);
    }

    Tensor Not(const Tensor & i_argument)
    {
        return MakeExpression(builtin_names::Not, {}, {i_argument});
    }

    Tensor Equal(const Tensor & i_first, const Tensor & i_second)
    {
        return MakeExpression(builtin_names::Equal, {}, {i_first, i_second});
    }

    Tensor Less(const Tensor & i_first, const Tensor & i_second)
    {
        return MakeExpression(builtin_names::Less, {}, {i_first, i_second});
    }

    Tensor Stack(Span<Tensor const> i_arguments)
    {
        return MakeExpression(builtin_names::Stack, {}, i_arguments);
    }

    Tensor Tuple(Span<Tensor const> i_arguments)
    {
        return MakeExpression(builtin_names::Tuple, {}, i_arguments);
    }

    Tensor If(Span<Tensor const> i_operands)
    {
        return MakeExpression(builtin_names::If, {}, i_operands);
    }

    Tensor MakeScope(Span<Tensor const> i_arguments)
    {
        return MakeExpression(builtin_names::Scope, {}, i_arguments);
    }

    bool AlwaysEqual(const Tensor & i_first, const Tensor & i_second)
    {
        return AlwaysEqual(*i_first.GetExpression(), *i_second.GetExpression());
    }

    Tensor SubstitutionAxiom(const Tensor & i_what, const Tensor & i_when, const Tensor & i_with)
    {
        return MakeExpression(builtin_names::SubstitutionAxiom, {}, {i_what, i_when, i_with});
    }

    bool IsConstant(const Tensor & i_tensor)
    {
        return i_tensor.GetExpression()->IsConstant();
    }

    bool IsIdentifier(const Tensor & i_tensor)
    {
        return NameIs(i_tensor, builtin_names::Identifier);
    }

    bool IsTensorType(const Tensor & i_tensor)
    {
        return NameIs(i_tensor, builtin_names::TensorType);
    }

    Tensor RepetitionsZeroToMany(Tensor i_tensor)
    {
        return MakeExpression(builtin_names::RepetitionsZeroToMany, {}, {i_tensor});
    }

    Tensor RepetitionsOneToMany(Tensor i_tensor)
    {
        return MakeExpression(builtin_names::RepetitionsOneToMany, {}, {i_tensor});
    }

    Tensor RepetitionsZeroToOne(Tensor i_tensor)
    {
        return MakeExpression(builtin_names::RepetitionsZeroToOne, {}, {i_tensor});
    }

    std::string ToSimplifiedStringForm(const Tensor & i_source)
    {
        StringBuilder builder;
        ToSimplifiedStringForm(builder, i_source);
        return builder.ShrinkAndGetString();
    }
}
