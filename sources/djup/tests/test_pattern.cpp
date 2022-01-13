
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/scope.h>
#include <core/diagnostic.h>

namespace djup
{
    namespace tests
    {
        void Pattern()
        {
            Print("Test: djup - Pattern Matching...");

            auto s = ToSimplifiedStringForm("0 * real"_t);

            R"(
                a = 4 + 6
            )"_t;

            DJUP_EXPECTS(Is("0"_t, "int"_t));
            DJUP_EXPECTS(Is("0"_t, "real"_t));
            DJUP_EXPECTS(!Is("true"_t, "int"_t));
            DJUP_EXPECTS(!Is("true"_t, "real"_t));
            DJUP_EXPECTS(!Is("1.2"_t, "int"_t));

            Scope scope(Scope::Root());
            scope.AddSubstitutionAxiom("2+3",           "5");
            scope.AddSubstitutionAxiom("0 * real",      "0");

            DJUP_EXPECTS(AlwaysEqual(scope.Canonicalize(Tensor("2+3")), Tensor("5")));
            DJUP_EXPECTS(AlwaysEqual(scope.Canonicalize(Tensor("0*7")), Tensor("0")));

            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup
