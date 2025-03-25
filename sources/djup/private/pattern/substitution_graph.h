
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <private/expression.h>
#include <private/pattern/discrimination_net.h>
#include <private/pattern/candidate.h>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>
#include <limits>
#include <functional>

namespace djup
{
    namespace pattern
    {
        class SubstitutionGraph
        {

        public:

            SubstitutionGraph(const DiscriminationNet & i_discrimination_net);

            ~SubstitutionGraph();

            void FindMatches(const Tensor& i_target, std::function<void()> i_step_callback = {});

            std::string ToDotLanguage(std::string_view i_graph_name) const;

        private:

            struct Node
            {
                uint32_t m_outgoing_edges{};
            };
            
            struct CandidateRef
            {
                uint32_t m_index = std::numeric_limits<uint32_t>::max();
                uint32_t m_version{};
            };

            struct Edge
            {
                uint32_t m_source_index{};
                CandidateRef m_candidate_ref; /** an edge candidate refers to its corresponding 
                    candidate if it's to be evaluated and confirmed yet. */
                uint32_t m_open{};
                uint32_t m_close{};
                std::vector<Substitution> m_substitutions;
            };
            
            class EdgeChain;

        private:

            void AddCandidate(uint32_t i_start_node, uint32_t i_end_node, 
                CandidateData i_new_candidate_data,
                std::vector<Substitution> && i_substitutions);

            void MatchCandidate(Candidate && i_candidate, std::function<void()> i_step_callback);

            bool MatchDiscriminationEdge(const Candidate & i_candidate, const DiscriminationNet::Edge & i_discrimination_edge);

            bool IsCandidateRefValid(CandidateRef i_ref) const;

            uint32_t NewNode();

            void RemoveNode(uint32_t i_node_index);

            void AddEdge(uint32_t i_start_node, uint32_t i_end_node, 
                CandidateRef i_candidate_ref, uint32_t i_open, uint32_t i_close,
                std::vector<Substitution>&& i_substitutions);

            void RemoveEdge(uint32_t i_start_node, uint32_t i_dest_node, CandidateRef i_candidate_ref);

        private:

            constexpr static uint32_t s_start_node_index = 0;
            constexpr static uint32_t s_end_node_index = 1;

            const DiscriminationNet & m_discrimination_net;

            /** Candidates are arranged in a stack because CandidateRef keeps the index of the referenced candidate. With stack, 
                popping the top of the stack does not shift the indices of the remaining candidates. Dangling indices in 
                CandidateRef are detected with a 'version' counter. */
            std::vector<Candidate> m_candidate_stack;
            
            /** Growable-only vector of nodes. Nodes are indentified by index, so no node can be evenr removed. */
            std::vector<Node> m_nodes;

            struct TerminalNode
            {
                uint32_t m_node_index;
                uint32_t m_pattern_id;
            };

            std::vector<TerminalNode> m_terminal_nodes;

            /** The key is the destination node */
            std::unordered_multimap<uint32_t, Edge> m_edges;

            /** Every candidate has a unique version. */
            uint32_t m_next_candidate_version{};

            // std::vector<Span<const Tensor>> m_tokens;
            // std::string m_tokens_str;

            #if DBG_CREATE_GRAPHVIZ_SVG
                std::string m_graph_name;
            #endif
        };
    
    } // namespace pattern

} // namespace djup
