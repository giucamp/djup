
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <core/from_chars.h>
#include <core/diagnostic.h>

namespace core
{
    namespace tests
    {
        void TestSystemUtils();
        void Traits();
        void ImmutableVector_();
        void Pool_();
        void Bits();
        void Memory();
        void ToChars();
        void FromChars();
        void ToString();
        void Split_();
        void TestUdpSocket();
        void GraphWiz();

        void Core()
        {
            PrintLn("Test: Core...");

            TestSystemUtils();
            ImmutableVector_();
            Pool_();
            GraphWiz();
            Traits();
            Bits();
            Memory();
            ToChars();
            FromChars();
            ToString();
            Split_();
            TestUdpSocket();

            PrintLn("successful");
        }

    } // namespace tests

} // namespace core
