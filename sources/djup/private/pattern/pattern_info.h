
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
        /** Inclusive range of integers. If the bounds are equal the range contains a single
            value. If the lower bound is greater than the upper bound the range is empty, 
            than this the values are meaningless. A default constructed range is empty. */
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

            // sums two ranges, possibly yielding to overflow
            Range operator + (const Range& i_other) const noexcept;

            // sums two ranges, possibly yielding to overflow
            Range& operator += (const Range& i_other) noexcept;

            /** returns whether the bounds of the ranges are identical */
            bool operator == (const Range & i_other) const noexcept;
            
            /** returns whether the bounds of the ranges not are identical */
            bool operator != (const Range & i_other) const noexcept;

            // clamps the input value to lay in this range
            int32_t ClampValue(int32_t i_value) const noexcept;

            // clamps the input range so that its values lay in this range
            Range ClampRange(Range i_range) const noexcept;

            // returns a string representation of the range, for example:
            // [1, 1], [0, 1], [0, Inf], [1, Inf]
            std::string ToString() const;
        };

        /** Describes a single parameter of a pattern, for example b in f(a, b, c) */
        struct ArgumentInfo
        {
            /** How many times this argument can be repeated: [1,1] for plain parameters,
                [0,Inf] for variadic parameters, etc. */
            Range m_cardinality;
            /** Given a parameter for this argument, how many parameters can follow. For
                example given f(a, b, c) and b is [1, 1], while for f(a, b..., c) is [1, Inf].
                This is redundant, but can early reject matching tries. s*/
            Range m_remaining;
        };

        /** Statically describes a pattern and its arguments, independently of
            the expressions it is tested against. */
        struct PatternInfo
        {
            FunctionFlags m_flags{}; //>** Associativity or commutativity of the pattern */
            /** Minimum and maximum number of parameters that may match this pattern */
            Range m_argument_range;
            /** Describes every single argument of the pattern. */
            std::vector<ArgumentInfo> m_arguments;
        };

        /** Returns true if the (root) expression is ?, .. or ... */
        bool IsRepetition(const Tensor& i_expression);

        /** Constructs a PatternInfo (static pattern information) given a pattern*/
        PatternInfo BuildPatternInfo(const Tensor & i_pattern);
    
    } // namespace pattern

} // namespace djup
