
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <djup/tensor.h>
#include <private/common.h>
#include <private/parser.h>
#include <tests/test_utils.h>

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

            real f(real t)
            {
                a = t + 2
                b = t^2 + a
                return a + b
            }                    
        }

)";
        void Parse()
        {
            Print("Test: djup - Parse...");

            Tensor namespace_1 = ParseExpression(Namespace1);

            ExpressionToGraph(*namespace_1.GetExpression()).SaveAsImage(
                GetArtifactPath("test_parse") / "namespace_1");

            Tensor v("2 + 3");
            
            Tensor v1("real [2 4] m");

            Tensor v2("a * a");

            // auto d = ToSimplifiedString("{{real}}x"_t);
            // auto s = ToSimplifiedString("real f( {{real}} x...)"_t);
            
            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup
