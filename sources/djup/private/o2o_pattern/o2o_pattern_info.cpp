
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <private/o2o_pattern/o2o_pattern_info.h>
#include <private/o2o_pattern/o2o_debug_utils.h>
#include <private/builtin_names.h>
#include <core/to_string.h>

namespace djup
{
    namespace o2o_pattern
    {
                        /**** Range ****/

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
            else if(!i_other.IsEmpty())
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
            else if(!i_other.IsEmpty())
            {
                m_min += i_other.m_min;
                if(m_min < i_other.m_min) // detects overflow
                    m_min = s_infinite;

                m_max += i_other.m_max;
                if(m_max < i_other.m_max) // detects overflow
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

        int32_t Range::ClampValue(int32_t i_value) const noexcept
        {
            if(i_value < m_min)
                return m_min;
            else if(i_value > m_max)
                return m_max;
            else
                return i_value;
        }

        Range Range::ClampRange(Range i_range) const noexcept
        {
            Range res = i_range;

            if(res.m_max > m_max)
                res.m_max = m_max;
            if(res.m_min < m_min)
                res.m_min = m_min;
            
            return res;
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


                    /**** BuildPatternInfo ****/

        namespace
        {
            Range GetCardinality(const Tensor & i_expression)
            {
                if (NameIs(i_expression, builtin_names::RepetitionsZeroToMany))
                    return { 0, Range::s_infinite };
                else if (NameIs(i_expression, builtin_names::RepetitionsZeroToOne))
                    return { 0, 1 };
                else if (NameIs(i_expression, builtin_names::RepetitionsOneToMany) || NameIs(i_expression, builtin_names::AssociativeIdentifier))
                    return { 1, Range::s_infinite };
                else
                    return { 1, 1 };
            }
        }

        bool IsRepetition(const Tensor& i_expression)
        {
            return NameIs(i_expression, builtin_names::RepetitionsZeroToMany)
                || NameIs(i_expression, builtin_names::RepetitionsZeroToOne)
                || NameIs(i_expression, builtin_names::RepetitionsOneToMany);
        }

        PatternInfo BuildPatternInfo(const Tensor & i_pattern)
        {
            Span<const Tensor> pattern_args = i_pattern.GetExpression()->GetArguments();

            PatternInfo result;
            result.m_flags = GetFunctionFlags(*i_pattern.GetExpression());

            result.m_arguments_info.resize(pattern_args.size());

            for(size_t i = 0; i < pattern_args.size(); i++)
            {
                result.m_arguments_info[i].m_kind = GetExpressionKind(*pattern_args[i].GetExpression());
                result.m_arguments_info[i].m_cardinality = GetCardinality(pattern_args[i]);
                result.m_arguments_range += result.m_arguments_info[i].m_cardinality;
            }

            for(size_t i = 0; i < pattern_args.size(); i++)
            {
                Range remaining{0, 0};
                for(size_t j = i + 1; j < pattern_args.size(); j++)
                    remaining += result.m_arguments_info[j].m_cardinality;
            
                result.m_arguments_info[i].m_remaining = remaining;
            }

            #if DJUP_DEBUG_PATTERN_INFO
                result.m_dbg_pattern = i_pattern;

                result.m_dbg_str_pattern = ToSimplifiedString(result.m_dbg_pattern);

                result.m_dbg_labels = "Arguments: " + result.m_arguments_range.ToString();
                for (size_t i = 0; i < result.m_arguments_info.size(); ++i)
                {
                    result.m_dbg_labels += "\nArg[" + std::to_string(i) + "]: [" +
                        result.m_arguments_info[i].m_cardinality.ToString();
                    result.m_dbg_labels += "], Remaining: " +
                        result.m_arguments_info[i].m_remaining.ToString();
                }
            #endif

            return result;
        }

        bool operator == (const PatternInfo & i_first, const PatternInfo & i_second)
        {
            if (i_first.m_arguments_range != i_second.m_arguments_range ||
                i_first.m_flags != i_second.m_flags ||
                i_first.m_arguments_info.size() != i_second.m_arguments_info.size())
            {
                return false;
            }

            for (size_t i = 0; i < i_first.m_arguments_info.size(); ++i)
            {
                if (i_first.m_arguments_info[i] != i_second.m_arguments_info[i])
                    return false;
            }

            return true;
        }

    } // namespace o2o_pattern

} // namespace djup

