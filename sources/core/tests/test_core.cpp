
#include <core/from_chars.h>
#include <core/diagnostic.h>

namespace djup
{
    namespace tests
    {
        void Traits();
        void ToChars();
        void FromChars();
        void ToString();
        void Split();
        void TestQueue();
        void TestUdpSocket();

        void Core()
        {
            PrintLn("Test: Core...");

            Traits();
            ToChars();
            FromChars();
            ToString();
            Split();
            TestQueue();
            TestUdpSocket();

            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup
