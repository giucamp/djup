
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <djup/tensor.h>
#include <private/name.h>
#include <private/expression.h>

namespace djup
{
    struct SubstitutionTarget
    {
        const Expression* m_expr{};
        size_t m_sequence_index{};
        bool operator == (const SubstitutionTarget & i_other) const
            { return m_expr == i_other.m_expr && m_sequence_index == i_other.m_sequence_index; }
    };

    struct SubstitutionTargetHash
    {
        size_t operator ()(const SubstitutionTarget & i_source) const
            { return i_source.m_expr->GetHash().ToSizeT() + i_source.m_sequence_index; }
    };

    struct PatternMatch
    {
        size_t m_pattern_id;
        std::unordered_map<SubstitutionTarget, Tensor, SubstitutionTargetHash> m_substitutions;
        std::unordered_map<const Expression*, size_t> m_expansions;
    };

    std::vector<PatternMatch> Match(const Tensor & i_target, const Tensor & i_pattern);

    Tensor SubstitutePatternMatch(const Tensor & i_source, const PatternMatch & i_match);
}
