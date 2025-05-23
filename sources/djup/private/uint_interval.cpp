
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <private/uint_interval.h>
#include <core/to_string.h>

namespace djup
{
    UIntInterval UIntInterval::operator | (const UIntInterval & i_other) const noexcept
    {
        UIntInterval res = *this;
        res |= i_other;
        return res;
    }

    UIntInterval & UIntInterval::operator |= (const UIntInterval & i_other) noexcept
    {
        if (IsEmpty())
        {
            *this = i_other;
        }
        else if (!i_other.IsEmpty())
        {
            m_min = std::min(m_min, i_other.m_min);
            m_max = std::max(m_max, i_other.m_max);
        }
        return *this;
    }

    UIntInterval UIntInterval::operator + (const UIntInterval & i_other) const noexcept
    {
        UIntInterval res = *this;
        res += i_other;
        return res;
    }

    UIntInterval & UIntInterval::operator += (const UIntInterval & i_other) noexcept
    {
        if (IsEmpty())
        {
            *this = i_other;
        }
        else if (!i_other.IsEmpty())
        {
            m_min += i_other.m_min;
            if (m_min < i_other.m_min) // detects overflow
                m_min = s_infinite;

            m_max += i_other.m_max;
            if (m_max < i_other.m_max) // detects overflow
                m_max = s_infinite;
        }
        return *this;
    }

    bool UIntInterval::operator == (const UIntInterval & i_other) const noexcept
    {
        return m_min == i_other.m_min && m_max == i_other.m_max;
    }

    bool UIntInterval::operator != (const UIntInterval & i_other) const noexcept
    {
        return !(*this == i_other);
    }

    uint32_t UIntInterval::ClampValue(uint32_t i_value) const noexcept
    {
        if (i_value < m_min)
            return m_min;
        else if (i_value > m_max)
            return m_max;
        else
            return i_value;
    }

    UIntInterval UIntInterval::ClampRange(UIntInterval i_range) const noexcept
    {
        UIntInterval res = i_range;

        if (res.m_max > m_max)
            res.m_max = m_max;
        if (res.m_min < m_min)
            res.m_min = m_min;

        return res;
    }

    std::string UIntInterval::ToString() const
    {
        if (IsEmpty())
            return "empty";
        else if (m_min == s_infinite && m_max == s_infinite)
            return djup::ToString("Inf, Inf");
        else if (m_min == s_infinite)
            return djup::ToString("Inf, ", m_max);
        else if (m_max == s_infinite)
            return djup::ToString(m_min, ", Inf");
        else
            return djup::ToString(m_min, ", ", m_max);
    }
}
