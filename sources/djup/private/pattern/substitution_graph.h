
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <private/expression.h>
#include <private/pattern/discrimination_tree.h>
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

            SubstitutionGraph(const DiscriminationTree & i_discrimination_net);

            ~SubstitutionGraph();

            void FindMatches(const Tensor& i_target, std::function<void()> i_step_callback = {});

            std::string ToDotLanguage(std::string_view i_graph_name) const;

        private:

            bool ProcessDiscriminationNodes(Span<const Tensor> i_targets,
                std::function<void()> i_step_callback);

            int32_t AddCandidate(Candidate&& i_candidate, const char* phase);

            int32_t MatchCandidate(int32_t i_parent_candidate, Candidate& i_candidate);

        private:

            const DiscriminationTree & m_discrimination_net;

            /** Candidates are arranged in a stack because CandidateRef keeps the index of the referenced candidate. With stack, 
                popping the top of the stack does not shift the indices of the remaining candidates. Dangling indices in 
                CandidateRef are detected with a 'version' counter. */
            std::vector<Candidate> m_candidate_stack;

            std::vector<uint32_t> m_discr_node_queue;
            std::vector<uint32_t> m_discr_node_to_substitution_node;

            struct Solution
            {
                std::vector<Substitution> m_substituitions;
            };

            std::vector<Solution> m_solutions;

            struct TerminalNode
            {
                uint32_t m_node_index;
                uint32_t m_pattern_id;
            };

            std::vector<TerminalNode> m_terminal_nodes;

            /** Every candidate has a unique version. */
            uint32_t m_next_candidate_version{};

            #if DBG_CREATE_GRAPHVIZ_SVG
                std::string m_graph_name;
            #endif
        };
    
    } // namespace pattern

} // namespace djup
