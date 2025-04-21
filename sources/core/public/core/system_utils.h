
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <filesystem> 

namespace core
{
    /** Returns the path of the executable file. Currently 
        implemented only on Windows and Linux (on other
        platforms the function is not defined). */
    std::filesystem::path GetExecutablePath();
}
