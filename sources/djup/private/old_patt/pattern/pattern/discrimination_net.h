
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <djup/tensor.h>
#include <private/old_pattern_match.h>
#include <private/pattern/pattern_info.h>
#include <unordered_map>

namespace djup
{
    namespace pattern
    {
        class DiscriminationNet
        {
        public:

            void AddPattern(uint32_t i_pattern_id, const Tensor & i_pattern, const Tensor & i_condition = true);

            uint32_t GetStartNode() const { return s_start_node_index; }
            
            /** In leaf edges m_expression is empty, and m_pattern_id is valid */
            struct Edge
            {
                std::vector<Tensor> m_patterns;
                Range m_cardinality;
                std::vector<ArgumentInfo> m_argument_infos;

                FunctionFlags m_function_flags{};
                bool m_is_leaf = false;

                union
                {
                    uint32_t m_dest_node;
                    uint32_t m_pattern_id;
                };
            };

            class EdgeSet
            {
            public:

                EdgeSet(DiscriminationNet const & i_net, uint32_t i_from_node)
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

            EdgeSet EdgesFrom(uint32_t i_from_node) const
            {
                return EdgeSet(*this, i_from_node);
            }

            std::string ToDotLanguage(std::string_view i_graph_name) const;

        private:

            constexpr static uint32_t s_start_node_index = 0;

            static bool SamePatterns(Span<const Tensor> i_first_patterns, Span<const Tensor> i_second_patterns);

            /** Returns the destination node */
            Edge * AddPatternFrom(uint32_t i_source_node, Span<const Tensor> i_patterns, 
                const PatternInfo & i_pattern_info);

            Edge * AddEdge(uint32_t i_source_node, Span<const Tensor> i_patterns);

            std::unordered_multimap<uint32_t, Edge> m_edges; /* The key is the source node index */
            
            uint32_t m_last_node_index = s_start_node_index;
        };

    } // namespace pattern

} // namespace djup
