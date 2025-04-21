
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <assert.h>
#include <type_traits>

namespace core
{
    /** Performs the bitwise for an enum type. This function can be used to implement 
        a bitwise or operator for an enum type:
        
        constexpr FLAGS operator | (FLAGS i_first, FLAGS i_second)
            { return CombineFlags(i_first, i_second); }

        where FLAGS is an enum type. */
    template <typename ENUM_TYPE, typename = std::enable_if_t<std::is_enum_v<ENUM_TYPE>>>
        constexpr ENUM_TYPE CombineFlags(ENUM_TYPE i_first, ENUM_TYPE i_second)
    {
        using UnderlyingType = std::underlying_type_t<ENUM_TYPE>;
        return static_cast<ENUM_TYPE>(static_cast<UnderlyingType>(i_first) | static_cast<UnderlyingType>(i_second));
    }

    /** Checks if a single bit is set in an enum type. If i_flag
        has more that one bit set, the behavior is undefined. */
    template <typename ENUM_TYPE, typename = std::enable_if_t<std::is_enum_v<ENUM_TYPE>>>
        constexpr bool HasFlag(ENUM_TYPE i_source, ENUM_TYPE i_flag)
    {
        using UnderlyingType = std::underlying_type_t<ENUM_TYPE>;
        const auto source = static_cast<UnderlyingType>(i_source);
        const auto flag = static_cast<UnderlyingType>(i_flag);

        // i_flag must have only one bit set
        assert(flag != 0 && (flag & (flag - 1)) == 0);

        return (source & flag) != 0;
    }

    /** Checks if all the bits in i_flags are set in i_source. 
        ENUM_TYPE must be an enum type. */
    template <typename ENUM_TYPE, typename = std::enable_if_t<std::is_enum_v<ENUM_TYPE>>>
        constexpr bool HasAllFlags(ENUM_TYPE i_source, ENUM_TYPE i_flags)
    {
        using UnderlyingType = std::underlying_type_t<ENUM_TYPE>;
        const auto source = static_cast<UnderlyingType>(i_source);
        const auto flags = static_cast<UnderlyingType>(i_flags);
        return (source & flags) == flags;
    }

    /** Checks if any of the bits in i_flags is set in i_source.
        ENUM_TYPE must be an enum type. */
    template <typename ENUM_TYPE, typename = std::enable_if_t<std::is_enum_v<ENUM_TYPE>>>
        constexpr bool HasAnyFlag(ENUM_TYPE i_source, ENUM_TYPE i_flags)
    {
        using UnderlyingType = std::underlying_type_t<ENUM_TYPE>;
        const auto source = static_cast<UnderlyingType>(i_source);
        const auto flags = static_cast<UnderlyingType>(i_flags);
        return (source & flags) != 0;
    }
}
