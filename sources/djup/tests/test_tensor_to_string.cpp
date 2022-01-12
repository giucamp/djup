
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <djup/tensor.h>
#include <core/diagnostic.h>

namespace djup
{
    namespace tests
    {
        void TensorToString()
        {
            Print("Test: djup - TensorToString...");

            Tensor a("real a");
            Tensor b("real b");
            DJUP_EXPECTS_EQ(ToSimplifiedStringForm(a + b * 2), 
                "Add(Identifier(TensorType(real, ), a), Mul(Identifier(TensorType(real, ), b), Literal(2)))");

            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup
