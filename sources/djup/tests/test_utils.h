
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <private/common.h>
#include <filesystem>
#include <core/diagnostic.h>

/** Parses the first and second, and then compares them. If they are not 
    equal, call Error (that launches an exception) the execution because 
    it means that a test has failed. */
#define DJUP_EXPR_EXPECTS_EQ(first, second) ::djup::tests::ExprExpectsEq(first, second, __FILE__, __LINE__)

namespace djup
{
    namespace tests
    {
        /** Returns (and create if not existing) the directory that
            should be used for files generated during testing. */
        const std::filesystem::path GetArtifactPath(std::string_view i_subdir);

        void ExprExpectsEq(
            std::string_view i_first,
            std::string_view i_second,
            const char* i_source_file, int i_source_line);
    }
}
