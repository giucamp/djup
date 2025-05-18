
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
        void M2oPatternInfo();
        void OldPattern();
        void TensorToGraph();
        void M2oDiscriminationTree_();
        void TestM2oSubstitutionBuilder();
        void O2oPattern();
        void M2oPattern();

        void Djup()
        {
            PrintLn("Test: djup...");

            Lexer();
            TensorToString();
            Parse();
            //M2oPatternInfo();
            OldPattern();
            TensorToGraph();
            //M2oDiscriminationTree_();
            //TestM2oSubstitutionBuilder();
            O2oPattern();
            //M2oPattern();

            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup
