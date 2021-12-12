
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <core/to_string.h>
#include <core/diagnostic.h>

namespace djup
{
    namespace tests
    {
        void ToString()
        {
            Print("Test: Core - ToString...");

            std::string dots(20000, '.');

            DJUP_EXPECTS_EQ(
                djup::ToString("abc", 1, "cde", dots, 1.5, "zzz"),
                "abc1cde" + dots + "1.5zzz");

            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup
