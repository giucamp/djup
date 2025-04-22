
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <djup/tensor.h>
#include <core/diagnostic.h>
#include <tests/test_utils.h>

namespace djup
{
    namespace tests
    {
        void TensorToString()
        {
            Print("Test: djup - TensorToString...");

            Tensor a("real a");
            Tensor b("real b");
            CORE_EXPECTS_EQ(ToSimplifiedString(a + b * 2), 
                "Add(Mul(2, real b), real a)");

            auto s = ToSimplifiedString("0 * real");
            CORE_EXPECTS(s == "Mul(0, real)"); 

            auto s1 = ToSimplifiedString("0 * Add(real x?, 4)..");
            CORE_EXPECTS(s1 == "Mul(0, Add(4, real x?))..");

            auto s2 = ToSimplifiedString("f((5 real a)...)");
            CORE_EXPECTS(s2 == "f((5, real a)...)");

            auto s3 = ToSimplifiedString("g((4 a)...)");
            CORE_EXPECTS(s3 == "g((4, a)...)");

            auto s4 = R"(
                (4 + 6) * (1 -5 * -4*1)
            )"_t;
            auto s4r = ToSimplifiedString(s4);
            //CORE_EXPECTS_EQ(s4r, "160");
            (void)s4r;

            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup
