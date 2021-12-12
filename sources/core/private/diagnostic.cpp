
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <core/diagnostic.h>
#ifdef _WIN32
    #include <Windows.h>
#endif

namespace djup
{
    namespace
    {
        thread_local int64_t g_silent_panic_count;

        class SilentErrorContext
        {
        public:

            SilentErrorContext();
            ~SilentErrorContext();

            SilentErrorContext(const SilentErrorContext &) = delete;
            SilentErrorContext & operator = (const SilentErrorContext &) = delete;
        };
    }

    SilentErrorContext::SilentErrorContext()
    {
        g_silent_panic_count++;
    }

    SilentErrorContext::~SilentErrorContext()
    {
        --g_silent_panic_count;
        if(g_silent_panic_count < 0)
            Error("Internak error: g_silent_panic_count is ", g_silent_panic_count);
    }

    void Expects(bool i_expr, const char * i_cpp_source_code)
    {
        if(!i_expr)
            Error("Expects - ", i_cpp_source_code, " is false");
    }

    void ExpectsError(const std::function<void()> & i_function,
        const char * i_cpp_source_code, const char * i_expected_message)
    {
        std::string panic_message;
        bool got_error = false;
        try
        {
            SilentErrorContext slient_panic;
            i_function();
        }
        catch (const std::exception & i_exc)
        {
            panic_message = i_exc.what();
            got_error = true;
        }
        catch (const StaticCStrException & i_exc)
        {
            panic_message = i_exc.c_str();
            got_error = true;
        }

        std::string message = "ExpectsError - ";
        message += i_cpp_source_code;

        if(!got_error)
            Error(message, " was supposed to Error");

        if(panic_message.find(i_expected_message) == std::string::npos)
            Error(message, " was supposed to Error with the message:\n",
                i_expected_message, ", but it gave:\n", panic_message);
    }

    void ExpectsError(const char * i_miu6_source_code, const char * i_expected_message)
    {
        ExpectsError(nullptr, i_miu6_source_code, i_expected_message);
    }

    [[noreturn]] void Error(const std::string & i_error)
    {
        if(g_silent_panic_count <= 0)
        {
            std::cerr << i_error << std::endl;

            #ifdef _WIN32
                if(IsDebuggerPresent())
                    DebugBreak();
            #endif
        }
        throw std::runtime_error(i_error.c_str());
    }

    void Print(const std::string & i_text)
    {
        std::cout << i_text;
    }
}
