
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/type.h>
#include <core/diagnostic.h>

namespace djup
{
    namespace tests
    {
        void Type()
        {
            Print("Test: djup - Type...");

            TensorType p1;
            TensorType p2(Domain::Complex, {});

            TensorType t1(Domain::Real, {});
            TensorType t2(Domain::Integer, {});

            static_assert(IsSupersetOf(Domain::Integer, Domain::Any));
            static_assert(IsSupersetOf(Domain::Integer, Domain::Complex));
            static_assert(IsSupersetOf(Domain::Integer, Domain::Real));
            static_assert(IsSupersetOf(Domain::Integer, Domain::Integer));

            static_assert(IsSupersetOf(Domain::Any, Domain::Any));

            DJUP_EXPECTS(TypeMatches(t1, p1));
            DJUP_EXPECTS(TypeMatches(t1, p2));
            DJUP_EXPECTS(TypeMatches(t2, p1));
            DJUP_EXPECTS(TypeMatches(t2, p2));

            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup
