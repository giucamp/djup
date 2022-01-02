
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <vector>
#include <limits>
#include <type_traits>
#include <core/hash.h>
#include <core/numeric_cast.h>
#include <private/name.h>
#include <private/type.h>

namespace djup
{
    class Tensor;
    class TensorType;

    class Expression
    {
    public:

        Expression(bool i_bool_literal);
        
        Expression(int64_t i_integer_literal);

        Expression(Name i_name, TensorType i_type, Span<const Tensor> i_arguments);

        const Name & GetName() const { return m_name; }

        Hash GetHash() const { return m_hash; }

        bool IsConstant() const { return m_is_constant; }

        bool IsBoolLiteral() const { return m_is_bool_literal; }

        bool IsIntegerLiteral() const { return m_is_integer_literal; }

        const std::vector<Tensor> & GetArguments() const { return m_arguments; }

    private:
        Hash m_hash;
        bool m_is_constant = false;
        bool m_is_integer_literal = false;
        bool m_is_bool_literal = false;
        Name m_name;
        TensorType m_type;
        std::vector<Tensor> m_arguments;
    };

    inline Hash & operator << (Hash & i_dest, const Expression & i_src)
    {
        return i_dest << i_src.GetHash();
    }

    Hash & operator << (Hash & i_dest, const Tensor & i_src);
   
    Tensor MakeExpression(Name i_name, TensorType i_type, Span<const Tensor> i_arguments);

    Tensor MakeExpression(Name i_name, Span<const Tensor> i_arguments);

    Tensor MakeLiteralExpression(bool i_bool_value);

    Tensor MakeLiteralExpression(int64_t i_integer_value);

    template <auto VALUE>
        const Tensor & MakeLiteralExpression()
    {
        if constexpr(std::is_same_v<decltype(VALUE), bool>)
        {
            static const Tensor s_value = MakeLiteralExpression(VALUE);
            return s_value;
        }
        else
        {
            static const Tensor s_value = MakeLiteralExpression(NumericCast<int64_t>(VALUE));
            return s_value;
        }
    }
}
