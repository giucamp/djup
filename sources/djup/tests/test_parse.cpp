
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
            g = 2
            h = 2
            real f(real t real w real p)
            {
                a = t + 2
                b = t^2 + a
                return real a + real b
            }                    
)";
        // constexpr char Namespace1[] = "a = 2";

        void Parse()
        {
            Print("Test: djup - Parse...");

            Tensor namespace_1 = ParseNamespace(Namespace1);

            auto s = ToSimplifiedString(namespace_1);

            TensorToGraph(namespace_1, {}).SaveAsImage(
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
