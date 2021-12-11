
#pragma once
#include <memory>
#include <string>
#include <variant>
#include <vector>
#include <core/hash.h>
#include <private/name.h>

namespace djup
{
    class Scope;

    class Expresssion
    {
    public:

        Expresssion(bool i_value);

        Expresssion(int64_t i_value);

        Expresssion(Name i_name, std::vector<std::shared_ptr<const Expresssion>> i_arguments);

        Expresssion(std::shared_ptr<const Scope> i_scope);

        uint64_t GetHash() const { return m_hash; }

    private:
        
        struct SymbolRef
        {
            Name m_name;
            std::vector<std::shared_ptr<const Expresssion>> m_arguments;
        };
        
        struct IntegerConstant
        {
            int64_t m_value;
        };

        struct BoolConstant
        {
            bool m_value;
        };

        struct ScopeExpression
        {
            std::shared_ptr<const Scope> m_scope;
        };

        std::variant<SymbolRef, IntegerConstant, BoolConstant, ScopeExpression> m_content;
        uint64_t m_hash;
    };
}
