
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
    namespace pattern
    {
        class DiscriminationNet
        {
        public:

            using uint32_t = uint32_t;

            void AddPattern(uint32_t i_pattern_id, const Tensor & i_pattern, const Tensor & i_condition);

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
                uint32_t m_dest_node{0};
                FunctionFlags m_function_flags{};
            };

            constexpr static uint32_t s_start_node_index = 0;

            /** Returns the destination node */
            uint32_t AddPatternFrom(uint32_t i_pattern_id, uint32_t i_source_node, 
                const Tensor & i_pattern, const ArgumentInfo & i_argument_info,
                const Tensor & i_condition);

            Edge * AddEdge(uint32_t i_source_node, const Tensor & i_expression);

            std::unordered_multimap<uint32_t, Edge> m_edges; /* The key is the source node index */
            
            uint32_t m_last_node_index = s_start_node_index;
        };

    } // namespace pattern

} // namespace djup
