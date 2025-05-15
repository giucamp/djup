
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <private/common.h>
#include <private/m2o_pattern/m2o_substitutions_builder.h>
#include <core/pool.h>
#include <core/graph_wiz.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace djup
{
    class Namespace;

    namespace m2o_pattern
    {
        class DiscriminationTree;

        class SubstitutionGraph
        {

        public:

            enum class SolutionType
            {
                All,
                Any
            };

            SubstitutionGraph(const DiscriminationTree & i_discrimination_net, 
                SolutionType i_solution_type = SolutionType::All);

            ~SubstitutionGraph();

            void FindMatches(
                const Namespace& i_namespace, const Tensor& i_target,
                std::function<void()> i_step_callback = {});

            GraphWizGraph ToDotGraphWiz(std::string_view i_graph_name) const;

            struct Solution
            {
                uint32_t m_curr_node{ std::numeric_limits<uint32_t>::max() };
                int32_t m_pattern_id = -1;
                SubstitutionsBuilder m_substitutions;
            };

            size_t GetSolutionCount() const { return m_solutions.size(); }

            const Solution& GetSolutionAt(size_t i_index) const { return m_solutions[i_index]; }

            const std::vector<Solution>& GetSolutions() const { return m_solutions; }

        private:

            struct CandidateEdge
            {
                uint32_t m_source_node = std::numeric_limits<uint32_t>::max();
                uint32_t m_dest_node = std::numeric_limits<uint32_t>::max();
                uint32_t m_discrimination_node = std::numeric_limits<uint32_t>::max();

                Span<const Tensor> m_targets;

                uint32_t m_repetitions{ 1 };

                uint32_t m_open{ 0 };
                uint32_t m_close{ 0 };

                std::vector<Substitution> m_substitutions;
            };

            using CandHandle = Pool<CandidateEdge>::Handle;

            struct DescendContext
            {
                const Namespace & m_namespace;
            };

            /** Edge that may be a part of a solution-path, given that variable
                substitution in the path is not contradictory. */
            struct SolutionEdge
            {
                uint32_t m_dest{ std::numeric_limits<uint32_t>::max() };
                std::vector<Substitution> m_substitutions;
                uint32_t m_open{ 0 };
                uint32_t m_close{ 0 };
            };

        private:

            void ProcessCandidate(DescendContext & i_context, CandidateEdge i_candidate);

            uint32_t NewSolutionNode();

            void FlushSolutions();

        private:
            const DiscriminationTree & m_discrimination_tree;
            const SolutionType m_solution_type;

            // candidate edges
            Pool<CandidateEdge> m_candidate_edges;
            std::vector<CandHandle> m_candidate_edges_queue;
            std::unordered_set<uint32_t> m_reached_leaf_nodes;

            // solutions graph
            uint32_t m_solution_node_count{0};
            std::unordered_multimap<uint32_t, SolutionEdge> m_solution_graph;

            std::vector<Solution> m_solutions;
        };

        Tensor ApplySubstitutions(const Tensor & i_where,
            Span<const Substitution> i_substitutions);
    
    } // namespace m2o_pattern

} // namespace djup
