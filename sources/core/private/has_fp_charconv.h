
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

/* HAS_FLOAT_CHARCONV - detect the implementation of floating point functions
    of <charconv> https://en.cppreference.com/w/cpp/header/charconv based on 
    the compiler version */
#if defined(_MSC_VER) && _MSC_VER < 1518
    #define HAS_FLOAT_CHARCONV 0
#elif defined(__clang__)
    #define HAS_FLOAT_CHARCONV 0
#elif defined(__GNUC__) && !defined(__clang__) && __GNUC__ < 11
    #define HAS_FLOAT_CHARCONV 0
#else
    #define HAS_FLOAT_CHARCONV 1
#endif

