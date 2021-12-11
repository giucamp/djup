
#pragma once
#include <string>
#include <functional> // for std::hash
#include <core/hash.h>
#include <core/to_chars.h>

namespace djup
{
    class Name
    {
    public:

        Name(std::string i_name);

        const std::string & AsString() const { return m_name; }

        uint64_t GetHash() const { return m_hash; }

    private:
        std::string m_name;
        uint64_t m_hash;
    };

    template <> struct CharWriter<Name>
    {
        void operator() (CharBufferView & i_dest, const Name & i_source) noexcept
        {
            i_dest << i_source.AsString();
        }
    };

    inline Hash & operator << (Hash & i_dest, const Name & i_source)
    {
        return i_dest << i_source.GetHash();
    }
}

namespace std
{
    template <> struct hash<djup::Name>
    {
        size_t operator()(const djup::Name & i_source)
        {
            return std::hash<decltype(i_source.GetHash())>{}(i_source.GetHash());
        }
    };
}

