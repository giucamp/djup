
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <tests/test_utils.h>
#include <core/diagnostic.h>
#include <djup/tensor.h>
#include <core/system_utils.h>

#ifdef __linux
    #include <unistd.h>
    #include <sys/types.h>
    #include <pwd.h>
#endif

namespace djup
{
    namespace tests
    {

        const std::filesystem::path GetArtifactPath(std::string_view i_subdir)
        {
            #ifdef __linux__
                struct passwd* pw = getpwuid(getuid());
                const char* homedir = pw->pw_dir;
                std::filesystem::path dir = homedir;
            #else
                std::filesystem::path dir = GetExecutablePath().parent_path();
            #endif
            dir = dir / "artifacts/djup" / i_subdir;
            
            static bool result = std::filesystem::create_directories(dir);
            (void)result;
            return dir;
        }

        void ExprExpectsEq(
            std::string_view i_first,
            std::string_view i_second,
            const char* i_source_file, int i_source_line)
        {
            Tensor first(i_first), second(i_second);

            if (!(AlwaysEqual(first, second)))
            {
                Error(i_source_file, "(", i_source_line, ") - CORE_EXPECTS_EQ - first is:\n",
                    ToSimplifiedString(first), ", \nsecond is:\n",
                    ToSimplifiedString(second));
            }
        }

    } // namespace tests

} // namespace djup
