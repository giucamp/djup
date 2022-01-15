
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/namespace.h>

namespace djup
{
    namespace
    {
        std::shared_ptr<const Namespace> MakeStandardNamespace()
        {
            std::shared_ptr<Namespace> scope = std::make_shared<Namespace>("Standard", nullptr);
            scope->AddScalarType("bool", {});
            scope->AddScalarType("int", {});
            scope->AddScalarType("rational", {"int"});
            scope->AddScalarType("real", {"rational"});
            scope->AddScalarType("complex", {"real"});
            return scope;
        }
    }

    std::shared_ptr<const Namespace> GetStandardNamespace()
    {
        static std::shared_ptr<const Namespace> standard_scope = MakeStandardNamespace();
        return standard_scope;
    }
}
