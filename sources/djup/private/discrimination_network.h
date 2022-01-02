
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <memory>
#include <unordered_map>
#include <limits>
#include <private/expression.h>

namespace djup
{
    class DiscriminationNetwork
    {
    public:

        DiscriminationNetwork();

        void AddPattern(size_t i_pattern_id, const Tensor & i_pattern, const Tensor & i_condition);


        struct Match
        {
            size_t m_pattern_id;
        };

        void FindMatches(const Tensor & i_target, std::vector<Match> o_matches) const;

    private:

        enum class EdgeKind
        {
            Constant, Variable, Name
        };

        struct Edge
        {
            size_t m_dest_node;
            EdgeKind m_kind;
            Expression m_expr;
        };

        constexpr static size_t s_terminal_dest_node = std::numeric_limits<size_t>::max();

        class LinearizedTarget;
        struct WalkingHead;

        std::unordered_multimap<size_t, Edge> m_edges;
        size_t m_next_node_index = 0;

    private:

        size_t AddSubPattern(size_t i_node_index, size_t i_pattern_id, const Tensor & i_pattern, const Tensor & i_condition);

        bool MatchToken(WalkingHead & i_head, const LinearizedTarget & i_target) const;
    };
}
