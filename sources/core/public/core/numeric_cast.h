
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <core/diagnostic.h>
#include <type_traits>

namespace core
{
    template <typename DEST_TYPE, typename SOURCE_TYPE>
        DEST_TYPE NumericCast(SOURCE_TYPE i_source)
    {
        if constexpr(std::is_same_v<DEST_TYPE, SOURCE_TYPE>)
        {
            // source type and dest type are the same, no loss can occur
            return i_source;
        }
        else if constexpr(std::is_signed_v<DEST_TYPE> == std::is_signed_v<SOURCE_TYPE>)
        {
            // source type and dest type are both signed or unsigned, don't check the sign
            auto const result = static_cast<DEST_TYPE>(i_source);
            auto const source_back = static_cast<SOURCE_TYPE>(result);
            if(i_source != source_back)
                Error("NumericCast - Bad cast of ", i_source);
            return result;
        }
        else
        {
            // general case
            auto const result = static_cast<DEST_TYPE>(i_source);
            auto const source_back = static_cast<SOURCE_TYPE>(result);
            auto const source_negative = i_source < 0;
            auto const result_negative = result < 0;
            if(i_source != source_back || source_negative != result_negative)
                Error("NumericCast - Bad cast of ", i_source);
            return result;
        }
    }

} // namespace core
