
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <private/common.h>
#include <private/expression.h>
#include <private/o2o_pattern/o2o_debug_utils.h>
#include <vector>
#include <string>
#include <cstdint>
#include <limits>

/* Warning: this flags will alter the layout of classes, adding
   string where it is useful for debug purpose. They can also
   enable some Print. */
#define DJUP_DEBUG_PATTERN_INFO                         true

namespace djup
{
    namespace o2o_pattern
    {
        /** Describes a single parameter of a pattern, for example b in f(a, b, c) */
        struct ArgumentInfo
        {
            /** How many times this label can be repeated: [1,1] for plain parameters,
                [0,Inf] for variadic parameters, etc. */
            IntInterval m_cardinality;

            /** Given a parameter for this argument, how many parameters can follow. For
                example given f(a, b, c) and b is [1, 1], while for f(a, b..., c) is [1, Inf].
                This is redundant, but can early reject matching tries. s*/
            IntInterval m_remaining;

            //** Constant, identifier, variadic or variable function. */
            ExpressionKind m_kind{};

            friend bool operator == (const ArgumentInfo & i_first, const ArgumentInfo & i_second)
            {
                return i_first.m_cardinality == i_second.m_cardinality &&
                    i_first.m_remaining == i_second.m_remaining&&
                    i_first.m_kind == i_second.m_kind;
            }

            friend bool operator != (const ArgumentInfo & i_first, const ArgumentInfo & i_second)
            {
                return !(i_first == i_second);
            }
        };

        /** Statically describes a pattern and its arguments, independently of
            the expressions it is tested against. To do: encapsulate as an
            immutable class */
        struct PatternInfo
        {
            #if DJUP_DEBUG_PATTERN_INFO
                std::string m_dbg_str_pattern;
                std::string m_dbg_labels;
                Tensor m_dbg_pattern;
            #endif

            FunctionFlags m_flags{}; //>** Associativity or commutativity of the pattern */

            /** Minimum and maximum number of parameters that may match this pattern.
                Used to early reject target spans. */
            IntInterval m_arguments_range;

            /** Describes every single label of the pattern. */
            std::vector<ArgumentInfo> m_arguments_info;

            friend bool operator == (const PatternInfo & i_first, const PatternInfo & i_second);

            friend bool operator != (const PatternInfo & i_first, const PatternInfo & i_second)
            {
                return !(i_first == i_second);
            }
        };

        /** Constructs a PatternInfo (static pattern information) given a pattern */
        PatternInfo BuildPatternInfo(const Tensor & i_pattern);

    } // namespace o2o_pattern

} // namespace djup

namespace core
{
    #if DJUP_DEBUG_PATTERN_INFO
    template <> struct CharWriter<djup::o2o_pattern::PatternInfo>
    {
        void operator() (CharBufferView& i_dest, const djup::o2o_pattern::PatternInfo & i_source)
        {
            i_dest << "Pattern: " << i_source.m_dbg_str_pattern << "\n";
            i_dest << "Arguments: " << i_source.m_arguments_range.ToString() << "\n";
            for (size_t i = 0; i < i_source.m_arguments_info.size(); ++i)
            {
                i_dest << "Label[" << i << "]: " << i_source.m_arguments_info[i].m_cardinality.ToString();
                i_dest << " Remaining: " << i_source.m_arguments_info[i].m_remaining.ToString() << "\n";
            }
        }
    };
    #endif

} //namespace core
