
#pragma once
#include <memory>
#include <core/span.h>

namespace djup
{
    class Tensor
    {
    public:

        Tensor(bool i_value);
        Tensor(int64_t i_value);
        Tensor(double i_value);

    public:

        Tensor(const std::shared_ptr<const class Expression> & i_expression)
            : m_expression(i_expression) { }

        Tensor(std::shared_ptr<const class Expression> && i_expression)
            : m_expression(std::move(i_expression)) { }

        const std::shared_ptr<const Expression> & GetExpression() const { return m_expression; }

    private:
        std::shared_ptr<const class Expression> m_expression;
    };

    Tensor Rank(const Tensor & i_tensor);

    Tensor Shape(const Tensor & i_tensor);

    bool IsConstant(const Tensor & i_tensor);

    bool Always(const Tensor & i_bool_tensor);

    bool Never(const Tensor & i_bool_tensor);

    Tensor Add(Span<Tensor const> i_addends);

    template <typename... TENSOR, typename = std::enable_if_t<
        (std::is_constructible_v<Tensor, TENSOR> && ...) >>
        Tensor Add(TENSOR && ... i_addends)
    {
        return Add({i_addends...});
    }

    Tensor Mul(Span<Tensor const> i_factors);

    template <typename... TENSOR, typename = std::enable_if_t<
        (std::is_constructible_v<Tensor, TENSOR> && ...) >>
        Tensor Mul(TENSOR && ... i_factors)
    {
        return Mul({i_factors...});
    }

    Tensor If(Span<Tensor const> i_operands);

    template <typename... TENSOR, typename = std::enable_if_t<
        (std::is_constructible_v<Tensor, TENSOR> && ...) &&
        sizeof...(TENSOR) % 2 == 1 >>
        Tensor If(TENSOR && ... i_operands)
    {
        return If({i_operands...});
    }

    Tensor And(Span<Tensor const> i_bool_operands);

    template <typename... TENSOR, typename = std::enable_if_t<
        (std::is_constructible_v<Tensor, TENSOR> && ...) >>
        Tensor And(TENSOR && ... i_bool_operands)
    {
        return And({i_bool_operands...});
    }

    Tensor Or(Span<Tensor const> i_bool_operands);

    template <typename... TENSOR, typename = std::enable_if_t<
        (std::is_constructible_v<Tensor, TENSOR> && ...) >>
        Tensor Or(TENSOR && ... i_bool_operands)
    {
        return Or({i_bool_operands...});
    }

    Tensor Not(Tensor const & i_bool_operand);

    Tensor operator + (const Tensor & i_operand);
    Tensor operator - (const Tensor & i_operand);

    Tensor operator + (const Tensor & i_first, const Tensor & i_second);
    Tensor operator - (const Tensor & i_first, const Tensor & i_second);
    Tensor operator * (const Tensor & i_first, const Tensor & i_second);
    Tensor operator / (const Tensor & i_dividend, const Tensor & i_divisor);

    Tensor operator && (const Tensor & i_first_bool, const Tensor & i_second_bool);
    Tensor operator || (const Tensor & i_first_bool, const Tensor & i_second_bool);
    Tensor operator ! (const Tensor & i_bool_operand);

    Tensor & operator += (Tensor & i_first, const Tensor & i_second);
    Tensor & operator -= (Tensor & i_first, const Tensor & i_second);
    Tensor & operator *= (Tensor & i_first, const Tensor & i_second);
    Tensor & operator /= (Tensor & i_dividend, const Tensor & i_divisor);

    Tensor operator < (const Tensor & i_first, const Tensor & i_second);
    Tensor operator > (const Tensor & i_first, const Tensor & i_second);
    Tensor operator <= (const Tensor & i_first, const Tensor & i_second);
    Tensor operator >= (const Tensor & i_first, const Tensor & i_second);
    Tensor operator == (const Tensor & i_first, const Tensor & i_second);
    Tensor operator != (const Tensor & i_first, const Tensor & i_second);

    Tensor Log(const Tensor & i_source);
    Tensor Exp(const Tensor & i_source);
    Tensor Pow(const Tensor & i_base, const Tensor & i_exponent);
    Tensor Square(const Tensor & i_source);
    Tensor Sin(const Tensor & i_operand);
    Tensor Cos(const Tensor & i_operand);

    Tensor Stack(Span<Tensor const> i_tensors);

    // to implement
    Tensor Substitute(const Tensor & i_where, const Tensor & i_what,
        const Tensor & i_with, const Tensor & i_when = true);

    struct Rule
    {
        Tensor const m_what, m_with, m_when = true;
    };

    Tensor Substitute(const Tensor & i_where, Span<const Rule> i_rules);

    std::ostream & operator << (std::ostream & i_dest, const Tensor & i_tensor);
}
