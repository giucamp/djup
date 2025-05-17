
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <private/common.h>
#include <core/to_chars.h>
#include <stdint.h>
#include <limits>
#include <string>

namespace djup
{
    /** Inclusive range of integers. If the bounds are equal the range contains a single
        value. If the lower bound is greater than the upper bound the range is empty,
        than this the values are meaningless. A default constructed range is empty.
        If min == max the range contains a single number.
        To do: rename to Interval */
    struct UIntInterval
    {
        uint32_t m_min{ 1 }; /**< inclusive lower bound */
        uint32_t m_max{ 0 }; /**< inclusive upper bound */

        constexpr static uint32_t s_infinite = std::numeric_limits<uint32_t>::max();

        /** returns whether no value is contained in this range */
        bool IsEmpty() const noexcept { return m_min > m_max; }

        /** returns whether a value is contained in this range */
        bool IsValaueWithin(uint32_t i_value) const noexcept
        {
            return i_value >= m_min && i_value <= m_max;
        }

        // makes this range containing all values of both input range
        UIntInterval operator | (const UIntInterval & i_other) const noexcept;

        // returns a range that contains both values of all input ranges
        UIntInterval & operator |= (const UIntInterval & i_other) noexcept;

        // sums two ranges, possibly yielding to infinity
        UIntInterval operator + (const UIntInterval & i_other) const noexcept;

        // sums two ranges, possibly yielding to an infinity
        UIntInterval & operator += (const UIntInterval & i_other) noexcept;

        /** returns whether the bounds of the ranges are identical */
        bool operator == (const UIntInterval & i_other) const noexcept;

        /** returns whether the bounds of the ranges not are identical */
        bool operator != (const UIntInterval & i_other) const noexcept;

        /** clamps the input value to lay in this range */
        uint32_t ClampValue(uint32_t i_value) const noexcept;

        /** clamps the input range so that its values lay in this range */
        UIntInterval ClampRange(UIntInterval i_range) const noexcept;

        /** Returns whether one and only on value lies in the range */
        bool HasSingleValue() const noexcept { return m_min == m_max; }

        // returns a string representation of the range, for example:
        // "1, 1", "0, 1", "2, Inf", "-2, Inf"
        std::string ToString() const;
    };

} // namespace djup

namespace core
{
    template <> struct CharWriter<djup::UIntInterval>
    {
        constexpr void operator() (CharBufferView & i_dest, const djup::UIntInterval & i_source)
        {
            const uint32_t infinite = djup::UIntInterval::s_infinite;

            if (i_source.m_min > i_source.m_max)
                i_dest << "empty";
            else if (i_source.m_min == infinite && i_source.m_max == infinite)
                i_dest << "Inf, Inf";
            else if (i_source.m_min == infinite)
                i_dest << "Inf, " << i_source.m_max;
            else if (i_source.m_max == infinite)
                i_dest << i_source.m_min << ", Inf";
            else
                i_dest << i_source.m_min << ", " << i_source.m_max;
        }
    };

} // namespace core
