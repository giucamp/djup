
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <djup/tensor.h>
#include <private/old_pattern_match.h>
#include <private/pattern/pattern_info.h>
#include <private/pattern/debug_utils.h>
#include <unordered_map>
#include <limits>

namespace djup
{
    namespace pattern
    {
        /** A discrimination tree represent a series o pattern to be tested against
            an expression (the target at the same time). */
        class DiscriminationTree
        {
        public:

            void AddPattern(uint32_t i_pattern_id, const Tensor & i_pattern,
                const Tensor & i_condition = true);
            
            /** In leaf edges m_expression is empty, and m_pattern_id is valid */
            struct Edge
            {
                PatternInfo m_pattern_info; // do do: debug info here are always empty
                std::vector<Tensor> m_labels;
                bool is_leaf_node{false};
                union
                {
                    uint32_t m_dest_node;
                    uint32_t m_pattern_id;
                }; 
                Edge() {}
            };

            static uint32_t GetRootNodeIndex() { return s_root_node_index; }

            const Edge* GetRootNode() const;

            const Edge & GetNodeFrom(uint32_t i_source_node) const;

            class EdgeSetIterator
            {
            public:

                EdgeSetIterator(DiscriminationTree const & i_net, uint32_t i_from_node)
                {
                    auto res = i_net.m_edges.equal_range(i_from_node);
                    m_begin = res.first;
                    m_end = res.second;
                }

                auto begin() const { return m_begin; }
                
                auto end() const { return m_end; }

            private:
                std::unordered_multimap<uint32_t, Edge>::const_iterator m_begin;
                std::unordered_multimap<uint32_t, Edge>::const_iterator m_end;
            };

            EdgeSetIterator EdgesFrom(uint32_t i_from_node) const
            {
                return EdgeSetIterator(*this, i_from_node);
            }

            int32_t GetNodeCount() const { return NumericCast<int32_t>(m_edges.size()); }

            /** Converts the discrimination net to a string processable with GraphWiz */
            std::string ToDotLanguage(std::string_view i_graph_name) const;

        private:

            constexpr static uint32_t s_root_node_index = 0;

            static bool SamePatterns(Span<const Tensor> i_first_patterns, Span<const Tensor> i_second_patterns);

            /** Returns the destination node */
            Edge * AddPatternFrom(uint32_t i_source_node,
                const Tensor& i_source, const PatternInfo & pattern_info);

            DiscriminationTree::Edge* ProcessPattern(
                int32_t i_source_node, Span<const Tensor> i_patterns, const PatternInfo& i_pattern_info);

            DiscriminationTree::Edge* AddEdge(
                uint32_t i_source_node, Span<const Tensor> i_patterns,
                const PatternInfo& i_source_pattern_info);

            std::unordered_multimap<uint32_t, Edge> m_edges; /* The key is the source node index */
            
            /** every node is identified by an index. INdices are not recycled.
                This is the last index assigned to a node. */
            uint32_t m_last_node_index = s_root_node_index;
        };

    } // namespace pattern

} // namespace djup
