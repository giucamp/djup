
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <djup/tensor.h>
#include <private/diagnostic.h>

namespace djup
{
    namespace tests
    {
        void Parse()
        {
            Print("Test: djup - Parse...");

            Tensor v("2 + 3");

            Tensor v1("real [2 4] m");

            "SubstitutionAxiom(real f(real x), , Scope(SubstitutionAxiom(a, , 1), SubstitutionAxiom(b, , 1), Stack(Add(a, b), 4)))"_t;

            DJUP_EXPR_EXPECTS_EQ("real f(real x) = { a = 1 b = 1 [a + b, 4] }",
                "SubstitutionAxiom(real f(real x), , Scope(SubstitutionAxiom(a, , 1), SubstitutionAxiom(b, , 1), Stack(Add(a, b), 4)))");

            // auto d = ToSimplifiedStringForm("{{real}}x"_t);
            // auto s = ToSimplifiedStringForm("real f( {{real}} x...)"_t);
            
            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup
