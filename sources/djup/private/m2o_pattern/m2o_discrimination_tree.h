
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <private/common.h>
#include <core/graph_wiz.h>
#include <djup/tensor.h>
#include <private/old_pattern_match.h>
#include <private/m2o_pattern/m2o_pattern_info.h>
#include <private/m2o_pattern/m2o_debug_utils.h>
#include <unordered_map>
#include <limits>

#define DJUP_DEBUG_DISCRIMINATION_TREE true

namespace djup
{
    namespace m2o_pattern
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
            
            static uint32_t GetRootNodeIndex() { return s_root_node_index; }

            /** Converts the discrimination net to a string processable with GraphWiz */
            GraphWizGraph ToGraphWiz(std::string_view i_graph_name) const;

            #if DJUP_DEBUG_DISCRIMINATION_TREE
                /*const auto & DbgGetFullPattern(uint32_t i_node) const
                {
                    const auto full_pattern_it = m_dbg_full_patterns.find(m_leaf_pattern_id[i_node]);
                    DJUP_ASSERT(full_pattern_it != m_dbg_full_patterns.end());
                    return full_pattern_it->second;
                }*/
            #endif

        private:

            struct Edge
            {
                Tensor m_pattern;
                PatternInfo m_pattern_info;
                std::vector<uint32_t> m_dest_nodes;
            };

            constexpr static uint32_t s_root_node_index = 0;

            void ProcessPattern(const Tensor & i_pattern);

            const Edge & AddEdje(uint32_t i_source_node, const Tensor & i_pattern);

            uint32_t NewNode();

        private:

            std::unordered_multimap<uint32_t, Edge> m_edges;

            uint32_t m_node_count{};
            
            #if DJUP_DEBUG_DISCRIMINATION_TREE
                // the key is the pattern id
                std::unordered_map<uint32_t, std::string> m_dbg_full_patterns;
            #endif
        };

    } // namespace m2o_pattern

} // namespace djup
