
#pragma once
#include <core/to_chars.h>
#include <utility>
#include <string>

namespace djup
{
    class StringBuilder
    {
    public:

        StringBuilder(size_t i_reserved_size = 511);

        friend void swap(StringBuilder & i_first, StringBuilder & i_second) noexcept;

        size_t size() const noexcept;

        const std::string & ShrinkAndGetString();

        template <typename TYPE> StringBuilder & operator<<(const TYPE & i_value)
        {
            static_assert(HasCharWriterV<TYPE>, "This type does not define a CharWriter");
            for (;;)
            {
                size_t const original_size = size();
                
                m_writer << i_value;
                if (!m_writer.IsTruncated())
                    return *this;

                Grow(original_size);
            }
        }

    private:        
        void Grow(size_t i_original_used_size);

    private:
        CharBufferView m_writer;
        std::string m_dest;
    };

    template <typename... TYPE> std::string ToString(const TYPE &... i_objects)
    {
        StringBuilder builder;
        (builder << ... << i_objects);
        return builder.ShrinkAndGetString();
    }

} // namespace edi
