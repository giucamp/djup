
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/scope.h>
#include <core/diagnostic.h>

namespace djup
{
    namespace tests
    {
        void Pattern()
        {
            Print("Test: djup - Pattern...");

            Scope scope(Scope::Root());
            scope.AddSubstitutionAxiom("2+3", "5", "true");

            DJUP_EXPECTS(AlwaysEqual( scope.Canonicalize(Tensor("2+3")), Tensor("5")));

            // DJUP_EXPECTS(TypeMatches(t1, p1));

            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup
