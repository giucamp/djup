
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <limits>
#include <vector>
#include <unordered_map>
#include <djup/tensor.h>
#include <private/expression.h>

namespace djup
{
    struct PatternMatch
    {
        size_t m_pattern_id;
        std::unordered_map<const Expression*, Tensor> m_substitutions;
        std::unordered_map<const Expression*, size_t> m_expansions;
    };

    Tensor SubstitutePatternMatch(const Tensor & i_source, const PatternMatch & i_match);

    class DiscriminationNet
    {
    public:

        DiscriminationNet();

        void AddPattern(size_t i_pattern_id, const Tensor & i_pattern, const Tensor & i_condition);

        void FindMatches(const Tensor & i_target, std::vector<PatternMatch> & o_matches) const;

    private:

        struct Edge
        {
            bool m_begin_arguments = false;
            bool m_end_arguments = false;
            bool m_is_terminal = false;
            Tensor m_expr;
            size_t m_dest_node;
        };

        constexpr static size_t s_terminal_dest_node = std::numeric_limits<size_t>::max();

        class LinearizedExpression;
        struct WalkingHead;

        std::unordered_multimap<size_t, Edge> m_edges;
        std::unordered_multimap<size_t, size_t> m_terminal_states;
        size_t m_next_node_index = 1;

    private:

        void MatchToken(std::vector<PatternMatch> & o_matches, std::vector<WalkingHead> & io_heads,
            const WalkingHead & i_curr_head, const LinearizedExpression & i_target) const;
    };
}
