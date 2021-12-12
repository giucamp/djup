
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/expression.h>

namespace djup
{
    Expression::Expression(bool i_value)
        : m_content(BoolConstant{i_value})
    {
        m_hash = ComputeHash(i_value);
    }

    Expression::Expression(int64_t i_value)
        : m_content(IntegerConstant{i_value})
    {
        m_hash = ComputeHash(i_value);
    }

    Expression::Expression(Name i_name, std::vector<std::shared_ptr<const Expression>> i_arguments)
        : m_content(SymbolRef{std::move(i_name), std::move(i_arguments)})
    {
        const SymbolRef & symbol_ref = std::get<SymbolRef>(m_content);

        Hash hash;
        hash << symbol_ref.m_name;
        hash << symbol_ref.m_arguments.size();
        for(const auto & argument : symbol_ref.m_arguments)
            hash << argument->GetHash();

        m_hash = hash.GetValue();
    }

    Expression::Expression(std::shared_ptr<const Scope> i_scope)
        : m_content(ScopeExpression{std::move(i_scope)})
    {

    }
}
