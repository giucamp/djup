
#pragma once
#include <core/span.h>
#include <core/to_chars.h>
#include <type_traits>
#include <string_view>
#include <string>

namespace djup
{
    // machine dependent hash
    class Hash
    {
    public:

        // Hash(values...)
        template <typename... TYPE>
            constexpr explicit Hash(const TYPE & ... i_object)
                { ((*this) << ... << i_object); }

        constexpr Hash & operator << (Span<const char> i_data)
        {
            // http://www.cse.yorku.ca/~oz/hash.html
            for(auto c : i_data)
                m_value = m_value * 33 + static_cast<uint64_t>(c);
            return *this;
        }

        constexpr auto GetValue() const    { return m_value; }

        constexpr bool operator == (const Hash & i_right) const
            { return m_value == i_right.m_value; }

        constexpr bool operator != (const Hash & i_right) const
            { return m_value != i_right.m_value; }

        constexpr bool operator < (const Hash & i_right) const
            { return m_value < i_right.m_value; }

        constexpr bool operator > (const Hash & i_right) const
            { return m_value > i_right.m_value; }

        constexpr bool operator <= (const Hash & i_right) const
            { return m_value <= i_right.m_value; }

        constexpr bool operator >= (const Hash & i_right) const
            { return m_value >= i_right.m_value; }

        using Word = uint64_t;

    private:
        Word m_value = 5381;
    };

    template <typename TYPE, typename = std::enable_if_t<
            std::is_arithmetic_v<TYPE> || std::is_enum_v<TYPE> >>
        Hash & operator << (Hash & i_dest, const TYPE & i_object)
    {
        // type-aliasing rule is not violated becuse we inspect the object with unsigned chars
        // https://en.cppreference.com/w/cpp/language/reinterpret_cast#Type_aliasing
        auto const address = reinterpret_cast<const unsigned char*>(&i_object);
        return i_dest << Span(address, sizeof(TYPE));
    }

    inline Hash & operator << (Hash & i_dest, const Hash & i_source)
    {
        return i_dest << i_dest.GetValue();
    }

    constexpr Hash & operator << (Hash & i_dest, std::string_view i_src)
    {
        return i_dest << Span(i_src.data(), i_src.length());
    }

    template <typename CONTAINER>
        FirstOf<Hash, ContainerElementTypeT<CONTAINER>> & operator << 
            (Hash & i_dest, const CONTAINER & i_container)
    {
        for(const auto & element : i_container)
            i_dest << element;
        return i_dest;
    }

    template <> struct CharWriter<Hash>
    {
        constexpr void operator() (CharBufferView & i_dest, Hash i_source) noexcept
        {
            i_dest << i_source.GetValue();
        }
    };
}
