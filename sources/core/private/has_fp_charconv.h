
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

#if !HAS_FLOAT_CHARCONV
    #pragma message(__FILE__ ": based on the compiler version support for floating point functions of <charconv> has been deduced as absent")
#endif
