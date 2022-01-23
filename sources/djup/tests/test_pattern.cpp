
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/namespace.h>
#include <core/diagnostic.h>

namespace djup
{
    namespace tests
    {
        void Pattern()
        {
            Print("Test: djup - Pattern Matching...");

            // Match("5"_t, "real x"_t);

            // auto m1 = Match("f(1, 2, 3, 4, 5, 6, 7, 8)"_t, "f(1, 2, real x..., 6, 7, 8)"_t);

            auto m1 = Match("f(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14)"_t, 
                "f(1, 2, real x..., 6, 7, 8, real y..., 12, 13, 14, 15)"_t);

            Tensor t = "1";

            auto s = ToSimplifiedStringForm("0 * real");

            auto s1 = ToSimplifiedStringForm("0 * Add(real x?, 4)..");

            auto s2 = ToSimplifiedStringForm("f((5 real a)...)");

            auto s3 = ToSimplifiedStringForm("g(...(4 a))");

            R"(
                a = 4 + 6
            )"_t;

            DJUP_EXPECTS(Is("0", "int"));
            DJUP_EXPECTS(Is("0", "real"));
            DJUP_EXPECTS(Is("false", "bool"));
            DJUP_EXPECTS(!Is("true", "int"));
            DJUP_EXPECTS(!Is("true", "real"));
            DJUP_EXPECTS(!Is("1.2", "int"));

            Namespace test_namespace("Test", Namespace::Root());
            test_namespace.AddSubstitutionAxiom("2+3",                  "5");
            test_namespace.AddSubstitutionAxiom("0 * real",             "0");
            test_namespace.AddSubstitutionAxiom("f((5, real a)...)",    "g(...(4, a))");


            DJUP_EXPECTS(AlwaysEqual(test_namespace.Canonicalize("2+3"), "5"));
            DJUP_EXPECTS(AlwaysEqual(test_namespace.Canonicalize("0*7"), "0"));

            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup
