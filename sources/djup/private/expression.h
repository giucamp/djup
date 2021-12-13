
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

        struct SymbolRef
        {
            Name m_name;
            Type m_type;
            std::vector<std::shared_ptr<const Expression>> m_arguments;
        };

        Expression(SymbolRef && i_symbol_ref);

        struct IntegerConstant
        {
            int64_t m_value;
        };

        Expression(IntegerConstant && i_integer_constant);

        struct BoolConstant
        {
            bool m_value;
        };

        Expression(BoolConstant && i_bool_constant);

        struct ScopeExpression
        {
            std::shared_ptr<const Scope> m_scope;
        };

        Expression(ScopeExpression && i_scope_expression);

        Hash GetHash() const { return m_hash; }

    private:
        std::variant<SymbolRef, IntegerConstant, BoolConstant, ScopeExpression> m_content;
        Hash m_hash;
    };

    inline Hash & operator << (Hash & i_dest, const Expression & i_src)
    {
        return i_dest << i_src.GetHash();
    }

    Hash & operator << (Hash & i_dest, const Tensor & i_src);
}
