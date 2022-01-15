
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
            std::shared_ptr<Namespace> std__namespace = std::make_shared<Namespace>("Standard", nullptr);
            std__namespace->AddScalarType("bool", {});
            std__namespace->AddScalarType("int", {});
            std__namespace->AddScalarType("rational", {"int"});
            std__namespace->AddScalarType("real", {"rational"});
            std__namespace->AddScalarType("complex", {"real"});
            return std__namespace;
        }
    }

    std::shared_ptr<const Namespace> GetStandardNamespace()
    {
        static std::shared_ptr<const Namespace> standard_namespace = MakeStandardNamespace();
        return standard_namespace;
    }
}
