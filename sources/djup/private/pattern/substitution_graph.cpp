
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <private/pattern/substitution_graph.h>
#include <private/pattern/discrimination_tree.h>
#include <private/pattern/pattern_info.h>
#include <private/builtin_names.h>
#include <algorithm>

namespace djup
{
    namespace pattern
    {
        SubstitutionGraph::SubstitutionGraph(const DiscriminationTree& i_discrimination_net)
            : m_discrimination_tree(i_discrimination_net), m_solution_node_count(i_discrimination_net.GetNodeCount())
        {
        }

        SubstitutionGraph::~SubstitutionGraph() = default;

        void SubstitutionGraph::FindMatches(const Tensor& i_target, std::function<void()> i_step_callback)
        {
            m_reached_leaf_nodes.clear();

            DescendContext context;
            context.m_step_callback = i_step_callback;

            // start by adding the discrimination root node as node to expand
            DiscrNodeToExpand root_node_to_expand;
            root_node_to_expand.m_node = DiscriminationTree::GetRootNodeIndex();
            root_node_to_expand.m_targets = { &i_target, 1 };
            m_discr_nodes_to_expand.push_back(root_node_to_expand);

            // main loop
            for(;;)
            {
                /* candidates have priority over node to expand to keep the
                   number of candidate low and access less memory. */
                if (!m_candidate_edges_queue.empty())
                {
                    const CandHandle cand_handle = m_candidate_edges_queue[0];
                    m_candidate_edges_queue.erase(m_candidate_edges_queue.begin());
                    ProcessCandidateEdge(cand_handle);
                    m_candidate_edges.Delete(cand_handle);
                }
                else if (!m_discr_nodes_to_expand.empty())
                {
                    const DiscrNodeToExpand node_to_expand = m_discr_nodes_to_expand.back();
                    m_discr_nodes_to_expand.pop_back();
                    ExpandDiscriminationmNode(node_to_expand);
                }
                else
                    break; // main loop is over
            }

            ProcessSolutions();
        }

        void SubstitutionGraph::ExpandDiscriminationmNode(
            const DiscrNodeToExpand & i_node_to_expand)
        {
            // expand the discrimination node
            for (auto& edge_it : m_discrimination_tree.EdgesFrom(i_node_to_expand.m_node))
            {
                const DiscriminationTree::Edge& edge = edge_it.second;

                /* early reject if the number of parameters (targets) is incompatible
                    with the number of labels in the pattern */
                if (edge.m_pattern_info.m_labels_range.IsValaueWithin(
                    NumericCast<int32_t>(i_node_to_expand.m_targets.size())))
                {
                    CandHandle handle = m_candidate_edges.New();
                    CandidateEdge& cand_edge = m_candidate_edges.GetObject(handle);

                    cand_edge.m_source_node = edge.m_dest_node;
                    cand_edge.m_dest_node = edge_it.first;

                    cand_edge.m_discr_edge = &edge;
                    cand_edge.m_targets = i_node_to_expand.m_targets;

                    m_candidate_edges_queue.push_back(handle);
                }
            }
        }

        void SubstitutionGraph::ProcessCandidateEdge(CandHandle i_candudate_edge_handle)
        {
            CandidateEdge * candidate = &m_candidate_edges.GetObject(i_candudate_edge_handle);

            const Span<const Tensor> labels =
                Span(candidate->m_discr_edge->m_labels).subspan(candidate->m_pattern_offset);
            const Span<const LabelInfo> label_infos =
                Span(candidate->m_discr_edge->m_pattern_info.m_labels_info).subspan(candidate->m_pattern_offset);

            DJUP_DEBUG_SUBSTGRAPH_PRINTLN("Process Candidate. Discr: ",
                candidate->m_source_node, " -> ", candidate->m_dest_node,
                ", rep: ", candidate->m_repetitions,
                "\n\ttargets: ", TensorSpanToString(candidate->m_targets),
                "\n\tlabels: ", TensorSpanToString(labels));

            uint32_t target_index = 0;
            for (uint32_t repetition = 0; repetition < candidate->m_repetitions; ++repetition)
            {
                for (uint32_t label_index = 0; label_index < labels.size(); ++label_index, ++target_index)
                {
                    const LabelInfo& pattern_info = label_infos[label_index];
                    if (pattern_info.m_cardinality.m_min == pattern_info.m_cardinality.m_max)
                    {
                        /* non-variadic argument */

                        const Tensor& label = labels[label_index];
                        const Tensor& target = candidate->m_targets[target_index];

                        if (target_index >= labels.size())
                        {
                            return;
                        }

                        if (IsConstant(label))
                        {
                            if (!AlwaysEqual(label, target))
                            {
                                return;
                            }
                        }
                        else if (NameIs(label, builtin_names::Identifier))
                        {
                            // check type
                            if (Is(target, label))
                            {
                                candidate->m_substitutions.push_back({ GetIdentifierName(label), target });
                            }
                            else
                            {
                                return;
                            }
                        }
                        else
                        {
                            // function call
                            if (label.GetExpression()->GetName() == target.GetExpression()->GetName())
                            {
                                DiscrNodeToExpand node_to_process;
                                node_to_process.m_targets = target.GetExpression()->GetArguments();
                                node_to_process.m_node = candidate->m_discr_edge->m_dest_node;
                                m_discr_nodes_to_expand.push_back(node_to_process);
                            }
                            else
                            {
                                return;
                            }
                        }
                    }
                    else
                    {
                        /* non-variadic argument */
                        DJUP_ASSERT(false);
                    }

                } // for each label
            } // for each repetition

            /* The pattern of this candidate is fully matched, so if the target
               is exhausted too the candidate is promoted to edge of the solution. */
            if (target_index == candidate->m_targets.size())
            {
                // candidate completed
                SolutionEdge& solution_edge = m_solution_graph[candidate->m_source_node];
                solution_edge.m_substitutions = std::move(candidate->m_substitutions);
                solution_edge.m_open = candidate->m_open;
                solution_edge.m_close = candidate->m_close;
                solution_edge.m_dest = candidate->m_dest_node;

                /*if (~candidate->m_source_discr_node != 0)
                {
                    // the parent of this candidate is a discrimination node
                    solution_edge.m_next_node = candidate->m_source_discr_node;
                }
                else
                {
                    DJUP_ASSERT(false);
                    // the parent of this candidate is another candidate
                    CandidateEdge& parent_candidate = m_candidate_edges.GetObject(candidate->m_parent_candidate);
                    DJUP_ASSERT(parent_candidate.m_source_discr_node != std::numeric_limits<uint32_t>::max());
                    solution_edge.m_next_node = parent_candidate.m_source_discr_node;
                }*/

                if (m_discrimination_tree.IsLeafNode(candidate->m_source_node))
                {
                    m_reached_leaf_nodes.insert(candidate->m_source_node);
                }
            }
        }

        /* Use m_solution_graph and m_reached_leaf_nodes to fill m_solutions 
           with actual solutions without contradictory substitutions. */
        void SubstitutionGraph::ProcessSolutions()
        {
            /* Create a solution for every reached leaf (=pattern) node.
               In this stage every solution is the beginning of zero, one 
               or more paths to the discrimination root (an actual solution). */
            m_solutions.clear();
            m_solutions.reserve(m_reached_leaf_nodes.size());
            for (uint32_t leaf_index : m_reached_leaf_nodes)
            {
                Solution candidate_solution;
                candidate_solution.m_curr_node = leaf_index;
                candidate_solution.m_pattern_id = m_discrimination_tree.GetPatternId(leaf_index);
                m_solutions.push_back(std::move(candidate_solution));
            }

            /* loop until all solutions still existing have not reached the root 
               node of the discrimination tree. When a node with more than one
               outgoing edges is found, the solution is cloned so that all paths
               can be followed. */
            bool any_processed;
            do {
                any_processed = false;

                /* iterate solutions by index because the vector may be altered during the loop */
                for (size_t solution_index = 0; solution_index < m_solutions.size(); )
                {
                    Solution & solution = m_solutions[solution_index];

                    // if not at the root of the discrimination tree root...
                    if (solution.m_curr_node != m_discrimination_tree.GetRootNodeIndex())
                    {
                        any_processed = true;

                        const size_t increment = ProcessSingleSolution(solution_index); // this may alter the vector
                        DJUP_ASSERT(sizeof(size_t) <= 4 || increment < std::numeric_limits<size_t>::max() / 2); /* if 
                            this fails probably an underflow has occurred */
                        solution_index += increment; 
                    }
                    else
                        ++solution_index;
                }

            } while (any_processed);
        }

        /* Duplicate solutions having more than one outgoing edges in m_solution_graph and
           remove the ones having no outgoing edges or having contradictory substitutions.*/
        size_t SubstitutionGraph::ProcessSingleSolution(size_t i_solution_index)
        {
            size_t solution_loop_increment = 1;

            const auto it_solution_edges_range = m_solution_graph.equal_range(m_solutions[i_solution_index].m_curr_node);
           
            // duplicate the solution for every additional outgoing edge
            bool any_outgoing_edge = false;
            for (auto it = it_solution_edges_range.first; it != it_solution_edges_range.second; it++)
            {
                if (any_outgoing_edge) // if not the first outgoing edge
                {
                    // make a copy of the solution
                    Solution solution_copy = m_solutions[i_solution_index];
                    m_solutions.insert(m_solutions.begin() + i_solution_index,
                        std::move(solution_copy));
                    solution_loop_increment++;
                }

                any_outgoing_edge = true;
            }

            if (!any_outgoing_edge)
            {
                // no outgoing edge, dead road
                m_solutions.erase(m_solutions.begin() + i_solution_index);

                DJUP_ASSERT(solution_loop_increment == 1);
                solution_loop_increment = 0;
            }
            else
            {
                for (auto it = it_solution_edges_range.first; it != it_solution_edges_range.second; it++)
                {
                    (void)std::remove_if(m_solutions.begin(), m_solutions.end(), 
                        [this, it, i_solution_index, &solution_loop_increment](Solution& i_solution) {

                        // to do: handle m_open and m_close
                        if (!AddSubstitutionsToSolution(i_solution.m_substitutions, it->second.m_substitutions))
                        {
                            m_solutions.erase(m_solutions.begin() + i_solution_index);

                            DJUP_ASSERT(solution_loop_increment > 0);
                            --solution_loop_increment;
                            return true;
                        }
                        else
                        {
                            i_solution.m_curr_node = it->second.m_dest;
                            return false;
                        }
                    });
                }
            }
            return solution_loop_increment;
        }

        /* Add the source substitutions to the dest vector. If there is
           a contradiction returns false, otherwise true. */
        bool SubstitutionGraph::AddSubstitutionsToSolution(
            std::vector<Substitution> & i_dest_substitutions,
            const std::vector<Substitution> & i_source_substitutions)
        {
            const size_t prev_substitution_count = i_dest_substitutions.size();

            // first add all the substitutions
            i_dest_substitutions.insert(i_dest_substitutions.end(),
                i_source_substitutions.begin(), i_source_substitutions.end());

            const size_t new_substitution_count = i_dest_substitutions.size();

            // quadratic search should be fine with a few substitutions
            // for every newly added substitution...
            for (size_t i = prev_substitution_count; i < new_substitution_count; ++i)
            {                
                for (size_t j = 0; j < i; ++j)
                {
                    if (i != j && i_dest_substitutions[i].m_variable_name == i_dest_substitutions[j].m_variable_name)
                    {
                        if (!AlwaysEqual(i_dest_substitutions[i].m_value, i_dest_substitutions[j].m_value))
                        {
                            return false;
                        }
                    }
                }
            }

            // no contradictions
            return true;
        }

        uint32_t SubstitutionGraph::NewVirtualNode()
        {
            return m_solution_node_count++;
        }

    } // namespace pattern

} // namespace djup
