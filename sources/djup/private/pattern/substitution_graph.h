
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <private/common.h>
#include <private/expression.h>
#include <private/pattern/discrimination_tree.h>
#include <private/pattern/candidate.h>
#include <private/pattern/debug_utils.h>
#include <core/pool.h>
#include <core/graph_wiz.h>
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

            GraphWizGraph ToDotGraphWiz(std::string_view i_graph_name) const;

        private:

            struct DescendContext
            {
                std::function<void()> m_step_callback;
            };

            struct Substitution
            {
                Name m_variable_name;
                Tensor m_value;
            };

            struct Candidate
            {
                /* Either m_parent_candidate or m_source_discr_node are valid */
                Pool<Candidate>::Handle m_parent_candidate;
                uint32_t m_source_discr_node = std::numeric_limits<uint32_t>::max();

                const DiscriminationTree::Edge* m_edge = nullptr;
                
                uint32_t m_dest_node; /** this can be an edge of the discrimination 
                    tree or a virtual node existing on the discrimination tree 
                    (virtual nodes). Virtual node indices starts after the last node
                    of the discrimination tree.*/

                Span<const Tensor> m_targets;
                size_t m_label_offset{};

                uint32_t m_repetitions{ 1 };

                uint32_t m_open{};
                uint32_t m_close{};

                uint32_t m_outcoming_edges{};

                std::vector<Substitution> m_substitutions;

                bool AddSubstitution(const Name& i_variable_name, const Tensor& i_value)
                {
                    m_substitutions.emplace_back(Substitution{ i_variable_name, i_value });
                    return true;
                }
            };

            using UInt = uint32_t;

            using CandHandle = Pool<Candidate>::Handle;

            struct DiscrNodeToProcess
            {
                uint32_t m_source_discr_node; /** index of the discrimination node to be expanded. */
                Span<const Tensor> m_targets;
            };

            struct DiscrNodeToProcess;

            Pool<Candidate> m_candidates;            

            void ExpandDiscrNode(
                DescendContext& i_context,
                int32_t i_discr_node,
                Span<const Tensor> i_targets);

            void ProcessCandidate(
                DescendContext& i_context,
                CandHandle i_candidate_handle);

            uint32_t NewVirtualNode() { return m_next_virtual_node++; }

        private:

            const DiscriminationTree & m_discrimination_tree;

            uint32_t m_next_virtual_node;

            std::vector<DiscrNodeToProcess> m_discr_node_stack;

            std::vector<CandHandle> m_pending_candidates;

            struct SolutionEdge
            {
                uint32_t m_next_node{std::numeric_limits<uint32_t>::max()};
                std::vector<Substitution> m_substitutions;
                uint32_t m_open{};
                uint32_t m_close{};
            };
            // the key is the 'lower' node
            std::unordered_map<uint32_t, SolutionEdge> m_solution_tree;

            struct Solution
            {
                std::vector<Substitution> m_substituitions;
            };

            std::vector<Solution> m_solutions;

            #if DBG_CREATE_GRAPHVIZ_SVG
                std::string m_graph_name;
            #endif
        };
    
    } // namespace pattern

} // namespace djup
