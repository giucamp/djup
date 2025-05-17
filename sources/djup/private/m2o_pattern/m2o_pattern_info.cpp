
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <private/m2o_pattern/m2o_pattern_info.h>
#include <private/m2o_pattern/m2o_debug_utils.h>
#include <private/builtin_names.h>
#include <core/to_string.h>

namespace djup
{
    namespace m2o_pattern
    {
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
                UIntInterval remaining{0, 0};
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

    } // namespace m2o_pattern

} // namespace djup

