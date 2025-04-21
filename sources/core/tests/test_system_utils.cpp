
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <core/system_utils.h>
#include <core/diagnostic.h>
#include <string_view>

namespace core
{
    namespace tests
    {
        void TestSystemUtils()
        {
            PrintLn("Test: Core - SystemUtils...");

            PrintLn("Exe: ", GetExecutablePath().string());

            PrintLn("successful");
        }

    } // namespace tests

} // namespace core
