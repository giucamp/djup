
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <assert.h>
#include <type_traits>

namespace core
{
    template <typename ENUM_TYPE, typename = std::enable_if_t<std::is_enum_v<ENUM_TYPE>>>
        constexpr ENUM_TYPE CombineFlags(ENUM_TYPE i_first, ENUM_TYPE i_second)
    {
        using UnderlyingType = std::underlying_type_t<ENUM_TYPE>;
        return static_cast<ENUM_TYPE>(static_cast<UnderlyingType>(i_first) | static_cast<UnderlyingType>(i_second));
    }

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

    template <typename ENUM_TYPE, typename = std::enable_if_t<std::is_enum_v<ENUM_TYPE>>>
        constexpr bool HasAllFlags(ENUM_TYPE i_source, ENUM_TYPE i_flags)
    {
        using UnderlyingType = std::underlying_type_t<ENUM_TYPE>;
        const auto source = static_cast<UnderlyingType>(i_source);
        const auto flags = static_cast<UnderlyingType>(i_flags);
        return (source & flags) == flags;
    }

    template <typename ENUM_TYPE, typename = std::enable_if_t<std::is_enum_v<ENUM_TYPE>>>
        constexpr bool HasAnyFlag(ENUM_TYPE i_source, ENUM_TYPE i_flags)
    {
        using UnderlyingType = std::underlying_type_t<ENUM_TYPE>;
        const auto source = static_cast<UnderlyingType>(i_source);
        const auto flags = static_cast<UnderlyingType>(i_flags);
        return (source & flags) != 0;
    }
}
