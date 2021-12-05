
#pragma once
#include <functional>
#include <variant>
#include <core/to_string.h>

namespace djup
{
    /** Exception described by a static null-terminated string. This class is 
        designed to be small, to be used with Expected without significantly 
        increasing its size. */
    class StaticCStrException
    {
    public:

        constexpr explicit StaticCStrException(const char * i_cstr_description)
            : m_cstr_description(i_cstr_description) { }

        constexpr const char * c_str() const noexcept
        {
            return m_cstr_description;
        }

    private:
        const char * m_cstr_description;
    };

    /** Holds a value or an error. Trying to get the value from an Expected
        that holds an error results in the error object to be thrown.
        If VALUE is void Expected is like an optional<ERROR>, and
        GetValue() can't be used. */
    template <typename VALUE, typename ERROR = StaticCStrException>
        class [[nodiscard]] Expected
    {
    public:

        template <typename... PARAMS>
            constexpr Expected(PARAMS &&... i_params)
                : m_content(std::forward<PARAMS>(i_params)...)
        {

        }

        constexpr explicit operator bool() const noexcept
        {
            return HasValue();
        }

        template <typename VAL = VALUE, typename = std::enable_if_t<!std::is_void_v<VAL>>>
            constexpr VAL & operator *()
        {
            return GetValue();
        }

        template <typename VAL = VALUE, typename = std::enable_if_t<!std::is_void_v<VAL>>>
            constexpr const VAL & operator *() const
        {
            return GetValue();
        }

        constexpr bool HasValue() const noexcept { return !std::holds_alternative<ERROR>(m_content); }

        constexpr bool HasError() const noexcept { return std::holds_alternative<ERROR>(m_content); }

        void TrowIfError()
        {
            if(HasError())
                throw std::get<ERROR>(m_content);
        }

        template <typename VAL = VALUE, typename = std::enable_if_t<!std::is_void_v<VAL>>>
            constexpr const VAL & GetValue() const
        {
            if(std::holds_alternative<VALUE>(m_content))
                return std::get<VALUE>(m_content);
            else
                throw std::get<ERROR>(m_content);
        }

        template <typename VAL = VALUE, typename = std::enable_if_t<!std::is_void_v<VAL>>>
            constexpr VAL & GetValue()
        {
            if(std::holds_alternative<VALUE>(m_content))
                return std::get<VALUE>(m_content);
            else
                throw std::get<ERROR>(std::move(m_content));
        }

        constexpr ERROR & GetError() noexcept
        {
            assert(HasError());
            return std::get<ERROR>(m_content);
        }

        constexpr const ERROR & GetError() const noexcept
        {
            assert(HasError());
            return std::get<ERROR>(m_content);
        }

    private:
        std::variant< 
            std::conditional_t<std::is_void_v<VALUE>, std::monostate, VALUE>, 
            ERROR > m_content;
    };

    void Print(const std::string & i_text);

    inline void PrintLn(const std::string_view & i_text)
    {
        Print(ToString(i_text, "\n"));
    }

    template <typename... TYPE>
        void Print(const TYPE & ... i_object)
    {
        Print(ToString(i_object...));
    }

    template <typename... TYPE>
        void PrintLn(const TYPE & ... i_object)
    {
        Print(ToString(i_object..., "\n"));
    }

    [[noreturn]] void Error(const std::string & i_error);

    template <typename... TYPE>
        [[noreturn]] void Error(const TYPE & ... i_object)
    {
        Error(ToString(i_object...));
    }

    void Expects(bool i_expr, const char * i_cpp_source_code);

    template <typename TYPE_1, typename TYPE_2>
        void ExpectsEq(
            const TYPE_1 & i_first, const char * i_first_cpp_expr,
            const TYPE_2 & i_second, const char * i_second_cpp_expr,
            const char * i_source_file, int i_source_line)
    {
        if(!(i_first == i_second))
            Error(i_source_file, "(", i_source_line, ") - SNABB_EXPECTS_EQ - ", 
                i_first_cpp_expr, " is ", i_first, ", ",
                i_second_cpp_expr, " is ", i_second );
    }

    void ExpectsError(const std::function<void()> & i_function,
        const char * i_cpp_source_code, const char * i_expected_message);

    #define SNABB_EXPECTS(expr) ::djup::Expects(expr, #expr)
    #define SNABB_EXPECTS_EQ(first, second) ::djup::ExpectsEq(first, #first, second, #second, __FILE__, __LINE__)

    #define SNABB_EXPECTS_ERROR(expr, expected_error) \
        ::djup::ExpectsError([&]{ (void)(expr); }, #expr, expected_error);
}
