
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <private/common.h>
#include <djup/expression.h>
#include <private/m2o_pattern/m2o_debug_utils.h>
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
    namespace m2o_pattern
    {
        /** Inclusive range of integers. If the bounds are equal the range contains a single
            value. If the lower bound is greater than the upper bound the range is empty, 
            than this the values are meaningless. A default constructed range is empty.
            If min == max the range contains a single number.
            To do: rename to Interval */
        struct Range
        {
            int32_t m_min{1}; /**< inclusive lower bound */
            int32_t m_max{0}; /**< inclusive upper bound */

            constexpr static int32_t s_infinite = std::numeric_limits<int32_t>::max();

            /** returns whether no value is contained in this range */
            bool IsEmpty() const noexcept { return m_min > m_max; }

            /** returns whether a value is contained in this range */
            bool IsValaueWithin(int32_t i_value) const noexcept 
                { return i_value >= m_min && i_value <= m_max; }

            // makes this range containing all values of both input range
            Range operator | (const Range & i_other) const noexcept;
            
            // returns a range that contains both values of all input ranges
            Range & operator |= (const Range & i_other) noexcept;

            // sums two ranges, possibly yielding to infinity
            Range operator + (const Range& i_other) const noexcept;

            // sums two ranges, possibly yielding to an infinity
            Range& operator += (const Range& i_other) noexcept;

            /** returns whether the bounds of the ranges are identical */
            bool operator == (const Range & i_other) const noexcept;
            
            /** returns whether the bounds of the ranges not are identical */
            bool operator != (const Range & i_other) const noexcept;

            /** clamps the input value to lay in this range */
            int32_t ClampValue(int32_t i_value) const noexcept;

            /** clamps the input range so that its values lay in this range */
            Range ClampRange(Range i_range) const noexcept;

            /** Returns whether one and only on value lies in the range */
            bool HasSingleValue() const noexcept { return m_min == m_max; }

            // returns a string representation of the range, for example:
            // "1, 1", "0, 1", "2, Inf", "-2, Inf"
            std::string ToString() const;
        };

        /** Describes a single parameter of a pattern, for example b in f(a, b, c) */
        struct ArgumentInfo
        {
            /** How many times this label can be repeated: [1,1] for plain parameters,
                [0,Inf] for variadic parameters, etc. */
            Range m_cardinality;

            /** Given a parameter for this argument, how many parameters can follow. For
                example given f(a, b, c) and b is [1, 1], while for f(a, b..., c) is [1, Inf].
                This is redundant, but can early reject matching tries. s*/
            Range m_remaining;

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
            Range m_arguments_range;

            /** Describes every single label of the pattern. */
            std::vector<ArgumentInfo> m_arguments_info;

            friend bool operator == (const PatternInfo & i_first, const PatternInfo & i_second);

            friend bool operator != (const PatternInfo & i_first, const PatternInfo & i_second)
            {
                return !(i_first == i_second);
            }
        };

        /** Returns true if the (root) expression is a repetition (?, .. or ...) */
        bool IsRepetition(const Tensor& i_expression);

        /** Constructs a PatternInfo (static pattern information) given a pattern */
        PatternInfo BuildPatternInfo(const Tensor & i_pattern);

    } // namespace m2o_pattern

} // namespace djup

namespace core
{
    template <> struct CharWriter<djup::m2o_pattern::Range>
    {
        constexpr void operator() (CharBufferView& i_dest, const djup::m2o_pattern::Range & i_source)
        {
            const int32_t infinite = djup::m2o_pattern::Range::s_infinite;

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

    #if DJUP_DEBUG_PATTERN_INFO
    template <> struct CharWriter<djup::m2o_pattern::PatternInfo>
    {
        void operator() (CharBufferView& i_dest, const djup::m2o_pattern::PatternInfo & i_source)
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
