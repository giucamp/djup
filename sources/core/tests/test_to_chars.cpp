
#include <core/to_chars.h>
#include <core/diagnostic.h>
#include <string_view>
#include <array>

namespace djup
{
    namespace tests
    {
        struct TypeWithNoToChars{};
        
        void ToChars()
        {
            Print("Test: Core - ToChars...");

            // HasCharWriterV
            static_assert(HasCharWriterV<bool>);
            static_assert(HasCharWriterV<int>);
            static_assert(HasCharWriterV<float>);
            static_assert(HasCharWriterV<std::nullptr_t>);
            static_assert(HasCharWriterV<int*>);
            static_assert(HasCharWriterV<const char*>);
            static_assert(!HasCharWriterV<TypeWithNoToChars>);

            // bool
            static_assert(std::string_view(ToCharArray<10>(true).data()) == "true");
            static_assert(std::string_view(ToCharArray<10>(false).data()) == "false");

            // ints
            static_assert(std::string_view(ToCharArray<10>(42u).data()) == "42");
            static_assert(std::string_view(ToCharArray<10>(-42).data()) == "-42");

            // floating point
            SNABB_EXPECTS_EQ(std::string_view(ToCharArray<10>(42.f).data()), "42");
            
            // sequence
            SNABB_EXPECTS_EQ(std::string_view(ToCharArray<128>(
                42.f, "abc", 52).data()), "42abc52");

            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup
