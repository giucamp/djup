
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <core/to_chars.h>
#include <core/diagnostic.h>
#include <string_view>
#include <vector>
#include <list>

namespace core
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
            static_assert(HasCharWriterV<std::string_view>);
            static_assert(!HasCharWriterV<TypeWithNoToChars>);

            // bool
            static_assert(std::string_view(ToCharArray<10>(true).data()) == "true");
            static_assert(std::string_view(ToCharArray<10>(false).data()) == "false");

            // ints
            static_assert(std::string_view(ToCharArray<10>(4u).data()) == "4");
            static_assert(std::string_view(ToCharArray<10>(-4).data()) == "-4");
            static_assert(std::string_view(ToCharArray<10>(42u).data()) == "42");
            static_assert(std::string_view(ToCharArray<10>(-42).data()) == "-42");
            static_assert(std::string_view(ToCharArray<10>(142u).data()) == "142");
            static_assert(std::string_view(ToCharArray<10>(-142).data()) == "-142");

            // floating point
            CORE_EXPECTS_EQ(std::string_view(ToCharArray<10>(42.f).data()), "42");

            // sequence
            CORE_EXPECTS_EQ(std::string_view(ToCharArray<128>(
                42.f, "abc", 52).data()), "42abc52");

            // containers
            {
                std::vector<int> ints{1, 2, 3};
                char out[64];
                CORE_EXPECTS_EQ(ToCharsView(out, ints), "1, 2, 3");
            }
            {
                std::list<int> ints{1, 2, 3};
                char out[64];
                CORE_EXPECTS_EQ(ToCharsView(out, ints), "1, 2, 3");
            }

            PrintLn("successful");
        }

    } // namespace tests

} // namespace core
