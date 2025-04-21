
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <core/system_utils.h>
#include <core/diagnostic.h>
#if defined(_WIN32)
    #include <windows.h>
    #include <vector>
#elif defined(__linux__)
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <unistd.h>
#endif

namespace core
{
    #if defined(_WIN32)
        std::filesystem::path GetExecutablePath()
        {
            std::vector<char> path_vec;

            uint32_t curr_buffer_length = 256;
            for (;;)
            {
                path_vec.resize(curr_buffer_length);
                DWORD result = GetModuleFileNameA(NULL, path_vec.data(), curr_buffer_length);
                if (result != curr_buffer_length)
                    break;
                if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
                    Error("GetExecutablePath - GetModuleFileName failed");

                curr_buffer_length *= 2;
            }
            
            return path_vec.data();
        }
    #elif defined(__linux__)
        std::filesystem::path GetExecutablePath()
        {
            const char path[] = "/proc/self/exe";
            return std::filesystem::canonical(path);
        }
    #endif
}
