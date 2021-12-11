
#include <core/from_chars.h>
#include <core/diagnostic.h>

namespace djup
{
    namespace tests
    {
        struct TypeWithNoParser { };

        void FromChars()
        {
            Print("Test: Core - FromChars...");

            // HasParserV
            static_assert(HasParserV<bool>);
            static_assert(HasParserV<int>);
            static_assert(HasParserV<float>);
            static_assert(!HasParserV<TypeWithNoParser>);

            // bool
            static_assert(Parse<bool>("true") == true);
            static_assert(Parse<bool>("false") == false);
            static_assert(!TryParse<bool>("abc"));
            DJUP_EXPECTS_ERROR(Parse<bool>(""), "Expected true or false");
            DJUP_EXPECTS_ERROR(Parse<bool>("cda"), "Expected true or false");

            // signed ints
            static_assert(Parse<int8_t>("-42") == -42);
            static_assert(Parse<int16_t>("-42") == -42);
            DJUP_EXPECTS_EQ(Parse<int32_t>("-42"), -42);
            DJUP_EXPECTS_EQ(Parse<int64_t>("-42"), -42);
            DJUP_EXPECTS(!TryParse<int8_t>(""));
            DJUP_EXPECTS(!TryParse<int8_t>("abc"));

            // unsigned ints
            static_assert(Parse<uint8_t>("42") == 42);
            static_assert(Parse<uint16_t>("42") == 42);
            DJUP_EXPECTS_EQ(Parse<uint32_t>("42"), uint32_t(42));
            DJUP_EXPECTS_EQ(Parse<uint64_t>("42"), uint64_t(42));

            // floating point
            DJUP_EXPECTS_EQ(Parse<float>("1.2"), 1.2f);
            DJUP_EXPECTS_EQ(Parse<double>("1.2"), 1.2);
            DJUP_EXPECTS_EQ(Parse<long double>("1.2"), 1.2L);
            DJUP_EXPECTS(!TryParse<float>("abc"));
            
            // sequence
            {
                std::string_view input("423trueabc");
                DJUP_EXPECTS_EQ(Parse<float>(input), 423);
                Accept(input, true);
                Accept(input, "abc");
                DJUP_EXPECTS(input.empty());
            }

            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup
