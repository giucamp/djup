
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <djup/tensor.h>

namespace djup
{
    struct PatternMatch
    {
        size_t m_pattern_id;
        std::unordered_map<const Expression*, Tensor> m_substitutions;
        std::unordered_map<const Expression*, size_t> m_expansions;
    };

    std::vector<PatternMatch> Match(const Tensor & i_target, const Tensor & i_pattern);

    Tensor SubstitutePatternMatch(const Tensor & i_source, const PatternMatch & i_match);
}
