
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
#ifndef NDEBUG
    #define DJUP_ASSERT(condition)          if(!(condition)) {DJUP_DEBUG_BREAK;} else (void)0
#else
    #if defined(_MSC_VER)
        #define DJUP_ASSERT(condition)      __assume(condition)
    #else
        #define DJUP_ASSERT(condition)      (void)0
    #endif
#endif

// DJUP_SUBST_GRAPH_DEBUG_PRINTLN - debug print for substitution graph
#define DJUP_DEBUG_SUBSTGRAPH_PRINTLN(...)          (void)0
//#define DJUP_DEBUG_SUBSTGRAPH_PRINTLN(...)        PrintLn(__VA_ARGS__)

// DJUP_DEBUG_DISCRTREE_PRINTLN - debug print for discrimination tree
#define DJUP_DEBUG_DISCRTREE_PRINTLN(...)           (void)0
// #define DJUP_DEBUG_DISCRTREE_PRINTLN(...)        PrintLn(__VA_ARGS__)

// introduce the namespace, otherwise the using would give error
namespace core { }

namespace djup
{
    using namespace core;
}