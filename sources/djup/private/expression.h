
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <memory>
#include <string>
#include <variant>
#include <vector>
#include <core/hash.h>
#include <private/name.h>
#include <private/type.h>

namespace djup
{
    class Scope;
    class Tensor;

    class Expression
    {
    public:

        struct TensorExpr
        {
            Name m_name;
            TensorType m_type;
            std::vector<Tensor> m_arguments;
        };

        Expression(TensorExpr && i_symbol_ref);

        bool IsTensorExpr() const { return std::holds_alternative<TensorExpr>(m_content); }

        const TensorExpr & AsTensorExpr() const { return std::get<TensorExpr>(m_content); }

        struct IntegerConstant
        {
            int64_t m_value;
        };

        Expression(IntegerConstant && i_integer_constant);

        bool IsIntegerConstant() const { return std::holds_alternative<IntegerConstant>(m_content); }

        struct BoolConstant
        {
            bool m_value;
        };

        Expression(BoolConstant && i_bool_constant);

        bool IsBoolConstant() const { return std::holds_alternative<BoolConstant>(m_content); }

        struct ScopeExpression
        {
            std::shared_ptr<const Scope> m_scope;
        };

        Expression(ScopeExpression && i_scope_expression);

        Hash GetHash() const { return m_hash; }

        bool IsConstant() const { return m_is_constant; }

    private:
        std::variant<TensorExpr, IntegerConstant, BoolConstant, ScopeExpression> m_content;
        Hash m_hash;
        bool m_is_constant = false;
    };

    inline Hash & operator << (Hash & i_dest, const Expression & i_src)
    {
        return i_dest << i_src.GetHash();
    }

    Hash & operator << (Hash & i_dest, const Tensor & i_src);

    template <auto VALUE>
        const Tensor & MakeConstant()
    {
        static Tensor s_value(std::make_shared<Expression>(Expression::IntegerConstant{VALUE}));
        return s_value;
    }

    Tensor MakeTensorExpression(const Name & i_name, Span<const Tensor> i_arguments);

    Tensor MakeTensorExpression(const Name & i_name, Span<const Tensor> i_arguments, const Scope & i_scope);
}
