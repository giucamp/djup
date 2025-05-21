
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <core/algorithms.h>
#include <core/diagnostic.h>
#include <vector>
#include <string>

namespace core
{
    namespace tests
    {
        void Algorithms()
        {
            Print("Test: Core - Algorithms...");

            std::vector<std::string> vect{"hello", "abc", "world"};
            std::string arr[] { "hello", "abc", "world" };

            CORE_EXPECTS(AnyOf(arr, [](auto s){ return s == "abc"; }));
            CORE_EXPECTS(AllOf(arr, [](auto s){ return s != "123"; }));

            CORE_EXPECTS(Contains(arr, "abc"));
            CORE_EXPECTS(!Contains(arr, "abc1"));

            CORE_EXPECTS(AnyOf(vect, [](auto s){ return s == "abc"; }));
            CORE_EXPECTS(AllOf(vect, [](auto s){ return s != "123"; }));

            CORE_EXPECTS(Contains(vect, "abc"));
            CORE_EXPECTS(!Contains(vect, "abc1"));

            PrintLn("successful");
        }

    } // namespace tests

} // namespace core
