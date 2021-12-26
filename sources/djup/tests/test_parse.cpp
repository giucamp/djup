
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
        void Parse()
        {
            Print("Test: Core - FromChars...");

            Tensor v("2 + 3");

            Tensor v1("real [2 4] m");

            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup
