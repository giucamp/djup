
#include <core/from_chars.h>
#include <private/has_fp_charconv.h>
#if HAS_FLOAT_CHARCONV
    #include <charconv>
#else
    #include <cstdlib>    
#endif

namespace djup
{
    #if HAS_FLOAT_CHARCONV

        namespace 
        {
            template <typename TYPE> Expected<TYPE> TryParseFloat(std::string_view & i_source) noexcept
            {
                TYPE out;
                std::from_chars_result const res = std::from_chars(
                    i_source.data(), i_source.data() + i_source.size(), out);
                if(res.ec == std::errc{})
                {
                    i_source.remove_prefix(res.ptr - i_source.data());
                    return out;
                } 
                else if(res.ec == std::errc::invalid_argument)
                {
                    return StaticCStrException("Unrecognized floating point value");
                }
                else if(res.ec == std::errc::result_out_of_range)
                {
                    return StaticCStrException("Floating point value out of range");
                }
                else
                {
                    return StaticCStrException("Unrecognized error while paring floating point");
                }
            }

        } // anonymous namespace

        Expected<float> Parser<float>::operator()(std::string_view & i_source) noexcept
        {
            return TryParseFloat<float>(i_source);
        }

        Expected<double> Parser<double>::operator()(std::string_view & i_source) noexcept
        {
            return TryParseFloat<double>(i_source);
        }

        Expected<long double> Parser<long double>::operator()(std::string_view & i_source) noexcept
        {
            return TryParseFloat<long double>(i_source);
        }

    #else // #if HAS_FLOAT_CHARCONV

        namespace
        {
            template <typename FLOAT_TYPE, typename STRTOF>
                Expected<FLOAT_TYPE> TryParseFloat(std::string_view & i_source, STRTOF i_strtof) noexcept
            {
                std::string source(i_source.data(), i_source.data() + i_source.size());

                char * end_of_number = nullptr;
                auto res = i_strtof(source.data(), &end_of_number);

                if (end_of_number != nullptr && end_of_number != source.data())
                {
                    assert(end_of_number >= source.data());
                    auto const read_chars = static_cast<size_t>(end_of_number - source.data());
                    i_source.remove_prefix(read_chars);
                    return res;
                }
                return StaticCStrException("Unexpected char while parsing floating point number");
            }

        } // anonymous namespace

        Expected<float> Parser<float>::operator()(std::string_view & i_source) noexcept
        {
            return TryParseFloat<float>(i_source, std::strtof);
        }

        Expected<double> Parser<double>::operator()(std::string_view & i_source) noexcept
        {
            return TryParseFloat<double>(i_source, std::strtod);
        }

        Expected<long double> Parser<long double>::operator()(std::string_view & i_source) noexcept
        {
            return TryParseFloat<long double>(i_source, std::strtold);
        }

    #endif // #if HAS_FLOAT_CHARCONV

} // namespace djup
