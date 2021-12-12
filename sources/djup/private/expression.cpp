
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/expression.h>
#include <djup/tensor.h>
#include <core/hash_variant.h>

namespace djup
{
    Hash & operator << (Hash & i_dest, const Expression::SymbolRef & i_src)
    {
        i_dest << i_src.m_name;
        i_dest << i_src.m_arguments;
        return i_dest;
    }

    Hash & operator << (Hash & i_dest, const Expression::BoolConstant & i_src)
    {
        i_dest << i_src.m_value;
        return i_dest;
    }

    Hash & operator << (Hash & i_dest, const Expression::IntegerConstant & i_src)
    {
        i_dest << i_src.m_value;
        return i_dest;
    }

    Hash & operator << (Hash & i_dest, const Expression::ScopeExpression & i_src)
    {
        Error("To do");
        return i_dest;
    }

    Expression::Expression(SymbolRef && i_symbol_ref)
        : m_content(std::move(i_symbol_ref))
    {
        m_hash << m_content;
    }

    Expression::Expression(IntegerConstant && i_integer_constant)
        : m_content(std::move(i_integer_constant))
    {
        m_hash << m_content;
    }

    Expression::Expression(BoolConstant && i_bool_constant)
        : m_content(std::move(i_bool_constant))
    {
        m_hash << m_content;
    }

    Expression::Expression(ScopeExpression && i_scope_constant)
        : m_content(std::move(i_scope_constant))
    {
        m_hash << m_content;
    }

    Hash & operator << (Hash & i_dest, const Tensor & i_src)
    {
        if(!i_src.IsEmpty())
            return i_dest << i_src.GetExpression()->GetHash();
        else
            i_dest << 1;
        return i_dest;
    }
}
