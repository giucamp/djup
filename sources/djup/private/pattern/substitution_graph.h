
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <private/common.h>
#include <private/pattern/discrimination_tree.h>
#include <core/pool.h>
#include <core/graph_wiz.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <functional>

namespace djup
{
    namespace pattern
    {
        /** Graph of substitutions that match a discrimination tree. 
            The substitution graph uses the same node indices as the discrimination tree,
            but it may have more nodes (virtual nodes), added to handle variadic arguments.
            The substitution graph can be walked backward with the respect of the discrimination
            graph (from the pattern nodes, that is the leaves), upward. SO the concept of
            source and destination node in the edge of the substitution tree is swapped.
            First discrimination nodes are added to a queue of nodes to be expanded. The
            algorithm is booted by adding the root of the discrimination tree.
            While expanding a discrimination node a candidate solution edge is created
            for every outgoing edge. During the main loop candidate edges are processed
            to check if they match the associated part of the target. If they match a 
            solution edge is added. In any case after processing the candidate edge is deleted. */
        class SubstitutionGraph
        {

        public:

            SubstitutionGraph(const DiscriminationTree & i_discrimination_net);

            ~SubstitutionGraph();

            void FindMatches(const Tensor& i_target, std::function<void()> i_step_callback = {});
            
            struct Substitution
            {
                Name m_variable_name;
                Tensor m_value;
            };

            struct Solution
            {
                uint32_t m_curr_node{ std::numeric_limits<uint32_t>::max() };
                int32_t m_pattern_id = -1;
                std::vector<Substitution> m_substitutions;
            };

            const std::vector<Solution> & GetSolutions() const { return m_solutions; }

            GraphWizGraph ToDotGraphWiz(std::string_view i_graph_name) const;

        private:

            struct DescendContext
            {
                std::function<void()> m_step_callback;
            };

            /** Discrimination node to be expanded to candidates */
            struct DiscrNodeToExpand
            {
                uint32_t m_node = std::numeric_limits<uint32_t>::max();
                Span<const Tensor> m_targets;
            };

            /** Edge that must be evaluated to be promoted to solution edge, or deleted */
            struct CandidateEdge
            {
                /* If m_source_candidate is a null handle, this candidate
                   is connected directly to a discrimination node. Otherwise
                   is connected to a candidate, which may be already been deleted */
                uint32_t m_source_node = std::numeric_limits<uint32_t>::max();
                //Pool<CandidateEdge>::Handle m_source_candidate;

                uint32_t m_dest_node = std::numeric_limits<uint32_t>::max();
                //Pool<CandidateEdge>::Handle m_dest_candidate;

                Span<const Tensor> m_targets;

                uint32_t m_pattern_offset{ 0 };

                uint32_t m_repetitions{ 1 };

                uint32_t m_open{ 0 };
                uint32_t m_close{ 0 };

                const DiscriminationTree::Edge* m_discr_edge{nullptr};

                std::vector<Substitution> m_substitutions;
            };
            using CandHandle = Pool<CandidateEdge>::Handle;

            /** Edge that may be a part of a solution-path, given that variable 
                substitution is not contradictory. */
            struct SolutionEdge
            {
                uint32_t m_dest{ std::numeric_limits<uint32_t>::max() };
                std::vector<Substitution> m_substitutions;
                uint32_t m_open{ 0 };
                uint32_t m_close{ 0 };
            };

        private:

            void ExpandDiscriminationmNode(const DiscrNodeToExpand& i_node_to_expand);

            void ProcessCandidateEdge(CandHandle i_candudate_edge_handle);

            void ProcessSolutions();

            size_t ProcessSingleSolution(size_t i_solution_index);

            bool AddSubstitutionsToSolution(std::vector<Substitution>& i_dest_substitutions,
                const std::vector<Substitution>& i_source_substitutions);

            uint32_t NewVirtualNode();

        private:

            const DiscriminationTree & m_discrimination_tree;

            // discrimination nodes to expand
            std::vector<DiscrNodeToExpand> m_discr_nodes_to_expand;

            // candidate edges
            Pool<CandidateEdge> m_candidate_edges;
            std::vector<CandHandle> m_candidate_edges_queue;
            uint32_t m_solution_node_count{ 0 };

            // solutions
            std::unordered_set<uint32_t> m_reached_leaf_nodes;
            std::unordered_map<uint32_t, SolutionEdge> m_solution_graph;
            std::vector<Solution> m_solutions;
        };
    
    } // namespace pattern

} // namespace djup
