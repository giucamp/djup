
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <core/diagnostic.h>

namespace djup
{
    namespace tests
    {
        void Lexer();
        void TensorToString();
        void Parse();
        void OldPattern();
        void Pattern();

        void Djup()
        {
            PrintLn("Test: djup...");

            Lexer();
            TensorToString();
            Parse();
            OldPattern();
            Pattern();

            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup
