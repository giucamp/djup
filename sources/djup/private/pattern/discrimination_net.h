
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <limits>
#include <unordered_map>
#include <djup/tensor.h>
#include <private/pattern_match.h>
#include <private/pattern/pattern_info.h>

namespace djup
{
    class DiscriminationNet
    {
    public:

        using NodeIndex = uint32_t;

        void AddPattern(NodeIndex i_pattern_id, const Tensor & i_pattern, const Tensor & i_condition);

        void FindMatches(const Tensor & i_target, std::vector<PatternMatch> & o_matches) const
        {

        }

        std::string ToDotLanguage(std::string_view i_graph_name) const;

    private:

        struct Edge
        {
            Tensor m_expression;
            Range m_cardinality;
            Range m_remaining_targets;
            Range m_argument_cardinality;
            NodeIndex m_dest_node{};
            FunctionFlags m_function_flags{};
        };

        struct AddPatternResult
        {
            NodeIndex m_dest_node_index;
            Range m_argument_cardinality;
        };

        AddPatternResult AddPatternFrom(NodeIndex i_pattern_id, NodeIndex i_from_node, 
            Span<const Tensor> i_patterns, const Tensor & i_condition);

        static Range GetCardinality(const Tensor & i_expression);

        Edge * AddEdge(NodeIndex i_source_node, const Tensor & i_expression);

        std::unordered_multimap<NodeIndex, Edge> m_edges; /* The key is the source node index */
        NodeIndex m_last_node_index = s_start_node_index;

        constexpr static NodeIndex s_start_node_index = 0;
        constexpr static NodeIndex s_max_node_index = std::numeric_limits<NodeIndex>::max();
        constexpr static NodeIndex s_terminal_dest_node = s_max_node_index;
    };
}
