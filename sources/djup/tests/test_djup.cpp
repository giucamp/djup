
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <tests/test_utils.h>

namespace djup
{
    namespace tests
    {
        void Lexer();
        void TensorToString();
        void Parse();
        void PatternInfo();
        void OldPattern();
        void TensorToGraph();
        void DiscriminationTree_();
        void Pattern();
        
        void Djup()
        {
            PrintLn("Test: djup...");

            Lexer();
            TensorToString();
            Parse();
            PatternInfo();
            OldPattern();
            TensorToGraph();
            DiscriminationTree_();
            Pattern();

            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup
