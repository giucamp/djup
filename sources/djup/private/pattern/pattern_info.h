
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <private/expression.h>
#include <vector>
#include <string>
#include <cstdint>
#include <limits>

namespace djup
{
    namespace pattern
    {
        struct Range
        {
            uint32_t m_min{1}; /**< inclusive lower bound */
            uint32_t m_max{0}; /**< inclusive upper bound */

            constexpr static uint32_t s_infinite = std::numeric_limits<uint32_t>::max();

            bool IsEmpty() const noexcept { return m_min > m_max; }

            Range operator + (const Range & i_other) const noexcept;
            Range & operator += (const Range & i_other) noexcept;

            Range operator | (const Range & i_other) const noexcept;
            Range & operator |= (const Range & i_other) noexcept;

            bool operator == (const Range & i_other) const noexcept;
            bool operator != (const Range & i_other) const noexcept;

            uint32_t Clamp(uint32_t i_value) const noexcept;

            Range Clamp(Range i_range) const noexcept;

            std::string ToString() const;
        };

        struct ArgumentInfo
        {
            Range m_cardinality;
            Range m_remaining;
        };

        struct PatternInfo
        {
            FunctionFlags m_flags;
            Range m_argument_range;
            std::vector<ArgumentInfo> m_arguments;
        };

        PatternInfo BuildPatternInfo(const Tensor & i_pattern);
    
    } // namespace pattern

} // namespace djup
