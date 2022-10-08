
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

            uint32_t GetStartNode() const { return s_start_node_index; }

            struct Edge
            {
                Tensor m_expression;
                ArgumentInfo m_info;
                Range m_argument_cardinality;
                uint32_t m_dest_node{0};
                FunctionFlags m_function_flags{};
            };

            class EdgeSet
            {
            public:

                EdgeSet(DiscriminationNet const & i_net, uint32_t i_from_node)
                    : m_net(i_net), m_from_node(i_from_node)
                {
                    auto res = m_net.m_edges.equal_range(i_from_node);
                    m_begin = res.first;
                    m_end = res.second;
                }

                auto begin() const { return m_begin; }
                
                auto end() const { return m_begin; }

            private:
                DiscriminationNet const & m_net;
                uint32_t const m_from_node;
                std::unordered_multimap<uint32_t, Edge>::const_iterator m_begin;
                std::unordered_multimap<uint32_t, Edge>::const_iterator m_end;
            };

            EdgeSet EdgesFrom(uint32_t i_from_node) const
            {
                return EdgeSet(*this, i_from_node);
            }

        private:

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
