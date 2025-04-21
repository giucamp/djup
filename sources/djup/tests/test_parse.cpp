
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <djup/tensor.h>
#include <private/common.h>
#include <private/parser.h>
#include <private/diagnostic.h>

namespace djup
{
    namespace tests
    {
        // Namespace  1
        constexpr char Namespace1[] = R"(
        {
            any a = 1
            real b = a * 2
            real [2 2] c = [ [1 2]
                             [3 4] ]                            
        }

)";
        void Parse()
        {
            Print("Test: djup - Parse...");

            Tensor namespace_1 = ParseExpression(Namespace1);

            ExpressionToGraph(*namespace_1.GetExpression()).SaveAsImage(
                GetArtifactPath() / "namespace_1.png");

            Tensor v("2 + 3");
            
            Tensor v1("real [2 4] m");

            Tensor v2("a * a");

            "SubstitutionAxiom(real f(real x), , Namespace(SubstitutionAxiom(any a, , 1), "
                "SubstitutionAxiom(any b, , 1), Stack(Add(a, b), 4)))"_t;

            DJUP_EXPR_EXPECTS_EQ("real f(real x) = { a = 1 b = 1 [a + b, 4] }",
                "SubstitutionAxiom(real f(real x), , Namespace(SubstitutionAxiom(a, , 1), SubstitutionAxiom(b, , 1), Stack(Add(a, b), 4)))");

            // auto d = ToSimplifiedStringForm("{{real}}x"_t);
            // auto s = ToSimplifiedStringForm("real f( {{real}} x...)"_t);
            
            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup
