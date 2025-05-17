
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <core/split.h>
#include <core/diagnostic.h>
#include <string>

namespace core
{
    namespace tests
    {
        void Split_()
        {
            Print("Test: Core - Split...");

            std::string expected_words[] = {"abc", "def", "fgh"};
            size_t i = 0;
            for(std::string_view word : Split("abc def fgh", ' '))
            {
                CORE_EXPECTS_EQ(expected_words[i], word);
                i++;
            }

            PrintLn("successful");
        }

    } // namespace tests

} // namespace core
