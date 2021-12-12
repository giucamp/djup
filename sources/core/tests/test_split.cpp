
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <core/split.h>
#include <core/diagnostic.h>
#include <string>

namespace djup
{
    namespace tests
    {
        void Split()
        {
            Print("Test: Core - Split...");

            std::string expected_words[] = {"abc", "def", "fgh"};
            size_t i = 0;
            for(auto && word : djup::Split("abc def fgh", ' '))
            {
                DJUP_EXPECTS_EQ(expected_words[i], word);
                i++;
            }

            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup
