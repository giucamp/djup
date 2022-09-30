
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/pattern/pattern_info.h>
#include <private/builtin_names.h>
#include <core/to_string.h>

namespace djup
{
    namespace pattern
    {
        Range Range::operator | (const Range & i_other) const noexcept
        {
            Range res = *this;
            res |= i_other;
            return res;
        }

        Range & Range::operator |= (const Range & i_other) noexcept
        {
            if(IsEmpty())
            {
                *this = i_other;
            }
            else
            {
                m_min = std::min(m_min, i_other.m_min);
                m_max = std::max(m_max, i_other.m_max);
            }
            return *this;
        }

        Range Range::operator + (const Range & i_other) const noexcept
        {
            Range res = *this;
            res += i_other;
            return res;
        }

        Range & Range::operator += (const Range & i_other) noexcept
        {
            if(IsEmpty())
            {
                *this = i_other;
            }
            else
            {
                m_min += i_other.m_min;
                if(m_min < i_other.m_min)
                    m_min = s_infinite;

                m_max += i_other.m_max;
                if(m_max < i_other.m_max)
                    m_max = s_infinite;
            }
            return *this;
        }

        bool Range::operator == (const Range & i_other) const noexcept
        {
            return m_min == i_other.m_min && m_max == i_other.m_max;
        }

        bool Range::operator != (const Range & i_other) const noexcept
        {
            return !(*this == i_other);
        }

        std::string Range::ToString() const
        {
            if(IsEmpty())
                return "empty";
            else if(m_min == s_infinite && m_max == s_infinite)
                return djup::ToString("Inf, Inf");
            else if(m_min == s_infinite)
                return djup::ToString("Inf, ", m_max);
            else if(m_max == s_infinite)
                return djup::ToString(m_min, ", Inf");
            else
                return djup::ToString(m_min, ", ", m_max);
        }

        Range GetCardinality(const Tensor & i_expression)
        {
            if(NameIs(i_expression, builtin_names::RepetitionsZeroToMany))
                return {0, Range::s_infinite};
            else if(NameIs(i_expression, builtin_names::RepetitionsZeroToOne))
                return {0, 1};
            else if(NameIs(i_expression, builtin_names::RepetitionsOneToMany) || NameIs(i_expression, builtin_names::AssociativeIdentifier))
                return {1, Range::s_infinite};
            else
                return {1, 1};
        }

        PatternInfo BuildPatternInfo(const Tensor & i_pattern)
        {
            Span<const Tensor> pattern_args = i_pattern.GetExpression()->GetArguments();

            PatternInfo result;
            result.m_flags = GetFunctionFlags(i_pattern.GetExpression()->GetName());

            result.m_arguments.resize(pattern_args.size());

            for(size_t i = 0; i < pattern_args.size(); i++)
            {
                result.m_arguments[i].m_cardinality = GetCardinality(pattern_args[i]);
                result.m_argument_range += result.m_arguments[i].m_cardinality;
            }

            for(size_t i = 0; i < pattern_args.size(); i++)
            {
                Range remaining{0, 0};
                for(size_t j = i + 1; j < pattern_args.size(); j++)
                    remaining += result.m_arguments[j].m_cardinality;
            
                result.m_arguments[i].m_remaining = remaining;
            }

            return result;
        }

    } // namespace pattern

} // namespace djup
