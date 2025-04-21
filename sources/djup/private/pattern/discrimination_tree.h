
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <private/common.h>
#include <core/graph_wiz.h>
#include <djup/tensor.h>
#include <private/old_pattern_match.h>
#include <private/pattern/pattern_info.h>
#include <private/pattern/debug_utils.h>
#include <unordered_map>
#include <limits>

#define DJUP_DEBUG_DISCRIMINATION_TREE false

namespace djup
{
    namespace pattern
    {
        /** A discrimination tree represent a series of patterns to be tested against
            a target expression. Patterns are identified by ids, whose value is picked
            by the user. */
        class DiscriminationTree
        {
        public:

            DiscriminationTree();

            // the pattern id must be positive
            void AddPattern(uint32_t i_pattern_id, const Tensor & i_pattern,
                const Tensor & i_condition = true);
            
            struct Edge
            {
                PatternInfo m_pattern_info; /* do do: debug info here are always 
                    incomplete because of merging multiple patterns */
                std::vector<Tensor> m_labels;
                uint32_t m_dest_node = std::numeric_limits<uint32_t>::max();
            };

            static uint32_t GetRootNodeIndex() { return s_root_node_index; }

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

            bool IsGraphEmpty() const { return m_leaf_pattern_id.empty(); }

            bool IsLeafNode(int32_t i_node_index) const { return m_leaf_pattern_id[i_node_index] != -1; }

            int32_t GetPatternId(int32_t i_node_index) const;

            /** Converts the discrimination net to a string processable with GraphWiz */
            GraphWizGraph ToGraphWiz(std::string_view i_graph_name) const;

        private:

            constexpr static uint32_t s_root_node_index = 0;

            static bool SameRootPatterns(Span<const Tensor> i_first_patterns, Span<const Tensor> i_second_patterns);

            DiscriminationTree::Edge* ProcessPattern(
                int32_t i_source_node, Span<const Tensor> i_patterns, const PatternInfo& i_pattern_info);

            DiscriminationTree::Edge* AddEdge(
                uint32_t i_source_node, Span<const Tensor> i_patterns,
                const PatternInfo& i_source_pattern_info);

            int32_t NewNode();

            /** Keys are the source node indices */
            std::unordered_multimap<uint32_t, Edge> m_edges; 

            /** Indicized by source nodes. Id of the pattern if the node is a leaf, -1 otherwise */
            std::vector<int32_t> m_leaf_pattern_id; 

            #if DJUP_DEBUG_DISCRIMINATION_TREE
                // the key is the pattern id
                std::unordered_map<int32_t, std::string> m_dbg_full_patterns;
            #endif
            
            int32_t m_node_count = 0;
        };

    } // namespace pattern

} // namespace djup
