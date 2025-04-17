
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

// DJUP_DEBUG_BREAK
#if defined(_MSC_VER)
    #define DJUP_DEBUG_BREAK    __debugbreak()
#elif defined(__clang__)
    #include <signal.h>
    #define DJUP_DEBUG_BREAK    raise(SIGTRAP)
#elif defined(__GNUC__)
    #include <signal.h>
    #define DJUP_DEBUG_BREAK    raise(SIGTRAP)
#else
    #error "missing implementation of DJUP_DEBUG_BREAK for this compiler"
#endif

// DJUP_ASSERT
#ifdef _DEBUG
    #define DJUP_ASSERT(condition)      if(!(condition)) {DJUP_DEBUG_BREAK;} else { }
#else
    #define DJUP_ASSERT(condition)      (void)0
#endif

// DJUP_SUBST_GRAPH_DEBUG_PRINTLN
// #define DJUP_DEBUG_SUBSTGRAPH_PRINTLN(...)     (void)0
#define DJUP_DEBUG_SUBSTGRAPH_PRINTLN(...)        PrintLn(__VA_ARGS__)