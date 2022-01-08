
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

            DJUP_EXPECTS_EQ(djup::ToString("abc", 1), "abc1");

            DJUP_EXPECTS_EQ(
                djup::ToString("abc", 1, "cde", dots, 1.5, "zzz"),
                "abc1cde" + dots + "1.5zzz");

            StringBuilder builder;
            builder.WriteLine();
            builder.Tab();
                builder.WriteLine("This is an int8_t: ", int8_t(5));
                builder.Tab();
                    builder.WriteLine("This is an uint8_t: ", uint8_t(5));
                builder.Untab();
                builder.WriteLine("This is a char: ", 'a');
            builder.Untab();
            std::string str = builder.ShrinkAndGetString();
            std::string test = "\n\tThis is an int8_t: 5\n\t\tThis is an uint8_t: 5\n\tThis is a char: a\n";
            DJUP_EXPECTS_EQ(str, test);

            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup
