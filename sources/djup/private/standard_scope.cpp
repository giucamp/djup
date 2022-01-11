
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/scope.h>

namespace djup
{
    namespace
    {
        std::shared_ptr<const Scope> MakeStandardScope()
        {
            std::shared_ptr<Scope> scope = std::make_shared<Scope>(nullptr);
            scope->AddScalarType("bool", {});
            scope->AddScalarType("int", {});
            scope->AddScalarType("rational", {"int"});
            scope->AddScalarType("real", {"rational"});
            scope->AddScalarType("complex", {"real"});
            return scope;
        }
    }

    std::shared_ptr<const Scope> GetStandardScope()
    {
        static std::shared_ptr<const Scope> standard_scope = MakeStandardScope();
        return standard_scope;
    }
}
