
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <private/common.h>
#include <core/diagnostic.h>
#include <djup/tensor.h>
#include <filesystem>
#include <core/system_utils.h>

namespace djup
{
    /** Returns (and create if not existing) the directory that 
        should be used for files generated during testing. */
    inline const std::filesystem::path GetArtifactPath()
    {
        const std::filesystem::path dir = GetExecutablePath().parent_path() / "artifacts";
        static bool result = std::filesystem::create_directory(dir);
        (void)result;
        return dir;
    }

    inline void ExprExpectsEq(
        std::string_view i_first, 
        std::string_view i_second,
        const char * i_source_file, int i_source_line)
    {
        Tensor first(i_first), second(i_second);

        if (!(AlwaysEqual(first, second)))
        {
            Error(i_source_file, "(", i_source_line, ") - CORE_EXPECTS_EQ - first is:\n",
                ToSimplifiedString(first), ", \nsecond is:\n", 
                ToSimplifiedString(second));
        }
    }

    /** Parses the first and second, and then compares them. If they are not 
        equal, call Error (that launches an exception) the execution because 
        it means that a test has failed. */
    #define DJUP_EXPR_EXPECTS_EQ(first, second) ::djup::ExprExpectsEq(first, second, __FILE__, __LINE__)
}
