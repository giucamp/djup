
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <memory>
#include <core/span.h>
#include <core/numeric_cast.h>

namespace djup
{
    class Expression;

    class Tensor
    {
    public:

        Tensor() = default;

        /** Construct a tensor from an integer, floating point or bool scalar constant */
        template <typename SCALAR, typename = std::enable_if_t<std::is_integral_v<SCALAR> || std::is_floating_point_v<SCALAR>>>
            Tensor(const SCALAR & i_scalar)
                : Tensor(ScalarConst{}, CanonicalizeScalar(i_scalar)) { }

        Tensor(std::string_view i_expression);
        
        bool IsEmpty() const noexcept { return m_expression.get() == nullptr; }

    public:

        Tensor(const std::shared_ptr<const Expression> & i_expression)
            : m_expression(i_expression) { }

        Tensor(std::shared_ptr<const Expression> && i_expression)
            : m_expression(std::move(i_expression)) { }

        const std::shared_ptr<const Expression> & GetExpression() const;

        std::shared_ptr<const Expression> StealExpression();

    private:

        enum class ScalarConst {};
        Tensor(ScalarConst, double i_scalar);
        Tensor(ScalarConst, int64_t i_scalar);
        Tensor(ScalarConst, bool i_scalar);

        template <typename TYPE>
            static auto CanonicalizeScalar(TYPE i_scalar)
        {
            if constexpr(std::is_same_v<TYPE, bool>)
                return i_scalar;
            else if constexpr(std::is_floating_point_v<TYPE>)
                return NumericCast<double>(i_scalar);
            else if constexpr(std::is_integral_v<TYPE>)
                return NumericCast<int64_t>(i_scalar);
        }

    private:
        std::shared_ptr<const Expression> m_expression;
    };

    inline Tensor operator ""_t(const char * i_source, size_t i_length)
    {
        return Tensor(std::string_view(i_source, i_length));
    }

    Tensor Rank(const Tensor & i_tensor);

    Tensor Shape(const Tensor & i_tensor);

    bool IsConstant(const Tensor & i_tensor);

    bool IsType(const Tensor & i_tensor);

    bool IsVariable(const Tensor & i_tensor);

    bool Always(const Tensor & i_bool_tensor);

    bool AlwaysEqual(const Tensor & i_first, const Tensor & i_second);

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

    Tensor Equal(const Tensor & i_first, const Tensor & i_second);
    
    Tensor Less(const Tensor & i_first, const Tensor & i_second);

    Tensor SubstitutionAxiom(const Tensor & i_what, const Tensor & i_when, const Tensor & i_with);

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

    Tensor Tuple(Span<Tensor const> i_arguments);

    Tensor Is(const Tensor & i_tensor, const Tensor & i_pattern);

    // to implement
    Tensor Substitute(const Tensor & i_where, const Tensor & i_what,
        const Tensor & i_with, const Tensor & i_when = true);

    struct Rule
    {
        Tensor const m_what, m_with, m_when = true;
    };

    Tensor Substitute(const Tensor & i_where, Span<const Rule> i_rules);

    template <> struct CharWriter<Tensor>
    {
        void operator() (CharBufferView & i_dest, const Tensor & i_source)
        {
            assert(false); // to do
        }
    };

    std::string ToSimplifiedStringForm(const Tensor & i_source);
}
