
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <core/from_chars.h>
#include <core/diagnostic.h>

namespace djup
{
    namespace tests
    {
        void Traits();
        void Bits();
        void Memory();
        void ToChars();
        void FromChars();
        void ToString();
        void Split();
        void TestUdpSocket();

        void Core()
        {
            PrintLn("Test: Core...");

            Traits();
            Bits();
            Memory();
            ToChars();
            FromChars();
            ToString();
            Split();
            TestUdpSocket();

            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup
