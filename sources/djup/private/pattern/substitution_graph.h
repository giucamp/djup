
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <private/expression.h>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>
#include <limits>

namespace djup
{
    namespace pattern
    {
        class DiscriminationNet;

        class SubstitutionGraph
        {

        public:

            void FindMatches(const DiscriminationNet & i_discrimination_net, 
                const Tensor & i_target, const Tensor & i_condition);

            std::string ToDotLanguage(std::string_view i_graph_name) const;

        private:

            struct Node;
            struct Candidate;
            struct Substitution;
            
            struct CandidateRef
            {
                uint32_t m_index = std::numeric_limits<uint32_t>::max();
                uint32_t m_version{};
            };

            struct Edge
            {
                uint32_t m_source_index{};
                CandidateRef m_candidate_ref;
                std::vector<Substitution> m_substitutions;
                uint32_t m_open{};
                uint32_t m_close{};
            };
            
            class LinearPath;
            friend class LinearPath;

        private:

            void AddCandidate( uint32_t i_start_node, uint32_t i_end_node,
                Span<const Tensor> i_targets, uint32_t i_discrimination_node,
                uint32_t i_open, uint32_t i_close, uint32_t i_repetitions = std::numeric_limits<uint32_t>::max());

            bool MatchCandidate(const DiscriminationNet & i_discrimination_net, Candidate & i_candidate);

            bool IsCandidateRefValid(CandidateRef i_ref) const;

            void RemoveNode(uint32_t i_node_index);

            void RemoveEdge(uint32_t i_start_node, uint32_t i_dest_node, CandidateRef i_candidate_ref);

        private:

            constexpr static uint32_t s_start_node_index = 0;
            constexpr static uint32_t s_end_node_index = 1;

            std::vector<Candidate> m_candidates;
            std::vector<Node> m_nodes;
            std::unordered_multimap<uint32_t, Edge> m_edges; // the key is the destination node
            uint32_t m_next_candidate_version{};
            #if DBG_CREATE_GRAPHVIZ_SVG
                std::string m_graph_name;
            #endif
        };
    
    } // namespace pattern

} // namespace djup
