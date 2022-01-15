
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

            Tensor t = "1";

            auto s = ToSimplifiedStringForm("0 * real");

            R"(
                a = 4 + 6
            )"_t;

            DJUP_EXPECTS(Is("0", "int"));
            DJUP_EXPECTS(Is("0", "real"));
            DJUP_EXPECTS(!Is("true", "int"));
            DJUP_EXPECTS(!Is("true", "real"));
            DJUP_EXPECTS(!Is("1.2", "int"));

            Namespace text_namespace("Test", Namespace::Root());
            //text_namespace.AddSubstitutionAxiom("2+3",           "5");
            text_namespace.AddSubstitutionAxiom("0 * real",      "0");

            //DJUP_EXPECTS(AlwaysEqual(text_namespace.Canonicalize(Tensor("2+3")), Tensor("5")));
            DJUP_EXPECTS(AlwaysEqual(text_namespace.Canonicalize("0*7"), "0"));

            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup
