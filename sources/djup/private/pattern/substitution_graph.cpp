
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/pattern/substitution_graph.h>
#include <private/pattern/discrimination_tree.h>
#include <private/pattern/candidate.h>
#include <private/pattern/pattern_info.h>
#include <private/builtin_names.h>
#include <core/flags.h>
#include <core/to_string.h>
#include <private/pattern/debug_utils.h>
#include <assert.h>

namespace djup
{
    namespace pattern
    {
        template< typename... ARGS>
            void SubstGraphDebugPrintLn(ARGS&&... args)
        {
            #if !defined(DJUP_DEBUG_PATTERN_MATCHING)
                #error DJUP_DEBUG_PATTERN_MATCHING must be defined
            #endif
            #if DJUP_DEBUG_PATTERN_MATCHING
                PrintLn(args...);
            #endif
        }

        SubstitutionGraph::SubstitutionGraph(const DiscriminationTree& i_discrimination_net)
            : m_discrimination_tree(i_discrimination_net),
              m_next_virtual_node(i_discrimination_net.GetNodeCount())
        {
        }

        SubstitutionGraph::~SubstitutionGraph() = default;

        void SubstitutionGraph::FindMatches(const Tensor& i_target, std::function<void()> i_step_callback)
        {
            SubstGraphDebugPrintLn();
            SubstGraphDebugPrintLn("------ FindMatches --------");

            if (m_discrimination_tree.IsGraphEmpty())
            {
                // empty discrimination tree
                return;
            }
            
            DescendContext context;
            context.m_step_callback = i_step_callback;

            if (context.m_step_callback)
                context.m_step_callback();

            // add root node for processing
            DiscrNodeToProcess root_node;
            root_node.m_source_discr_node = DiscriminationTree::GetRootNodeIndex();
            root_node.m_targets = { &i_target, 1 };
            m_discr_node_stack.push_back(root_node);

            do {

                // process candidates
                while (!m_pending_candidates.empty())
                {
                    CandHandle cand_handle = m_pending_candidates[0];
                    m_pending_candidates.erase(m_pending_candidates.begin());

                    ProcessCandidate(context, cand_handle);

                    m_candidates.Delete(cand_handle);
                }

                // process discrimination nodes
                while (!m_discr_node_stack.empty())
                {
                    DiscrNodeToProcess node = m_discr_node_stack.back();
                    m_discr_node_stack.pop_back();

                    if (m_discrimination_tree.IsLeafNode(node.m_source_discr_node))
                    {
                        // leaf node, MATCH!!!
                        return;
                    }

                    ExpandDiscrNode(context, node.m_source_discr_node,
                        node.m_targets, node.m_parent_candidate_handle);

                    if (context.m_step_callback)
                        context.m_step_callback();
                }

            } while (m_pending_candidates.size() + m_discr_node_stack.size() > 0);
        }

        void SubstitutionGraph::ExpandDiscrNode(
            DescendContext& i_context,
            int32_t i_source_discr_node,
            Span<const Tensor> i_targets,
            Pool<Candidate>::Handle i_parent_candidate)
        {
            // expand the discrimination source node
            for (auto& edge_it : m_discrimination_tree.EdgesFrom(i_source_discr_node))
            {
                const DiscriminationTree::Edge& edge = edge_it.second;

                /* early reject if the number of parameters (targets) is incompatible
                    with the number of labels in the pattern */
                if (edge.m_pattern_info.m_labels_range.IsValaueWithin(
                    NumericCast<int32_t>(i_targets.size())))
                {
                    CandHandle cand_handle = m_candidates.New();
                    Candidate& candidate = m_candidates.GetObject(cand_handle);
                    candidate.m_source_discr_node = i_source_discr_node;
                    candidate.m_edge = &edge;
                    // candidate.m_parent_candidate = i_parent_candidate; shouldn't be necessary
                    candidate.m_targets = i_targets;
                    
                    ProcessCandidate(i_context, cand_handle);
                    
                    m_candidates.Delete(cand_handle);
                }
            }
        }

        /* returns true if the candidate should be kept, false otherwise */
        void SubstitutionGraph::ProcessCandidate(
            DescendContext& i_context,
            const CandHandle & i_candidate_handle)
        {
            // extract for data in the candidate
            Candidate * candidate = &m_candidates.GetObject(i_candidate_handle);
            const Span<const Tensor> targets = candidate->m_targets;
            const Span<const Tensor> labels = 
                Span(candidate->m_edge->m_labels).subspan(candidate->m_label_offset);
            const Span<const LabelInfo> label_infos = 
                Span(candidate->m_edge->m_pattern_info.m_labels_info).subspan(candidate->m_label_offset);
            const UInt repetitions = candidate->m_repetitions;
            const uint32_t source_discr_node = candidate->m_source_discr_node;
            const DiscriminationTree::Edge& edge = *candidate->m_edge;
            const CandHandle parent_candidate_handle = candidate->m_parent_candidate;
            const uint32_t discr_dest_node = candidate->m_dest_node;
                // from now on candidate will be invalidated as soon as the pool is modified

            SubstGraphDebugPrintLn("Process Candidate. Discr: ", 
                source_discr_node, " -> ", edge.m_dest_node,
                ", rep: ", repetitions,
                "\n\ttargets: ", TensorSpanToString(targets), 
                "\n\tlabels: ", TensorSpanToString(labels));

            UInt target_index = 0;
            for (UInt repetition = 0; repetition < repetitions; ++repetition)
            {
                for (UInt label_index = 0; label_index < labels.size(); 
                    ++label_index, ++target_index)
                {
                    const LabelInfo & label_info = label_infos[label_index];
                    if (label_info.m_cardinality.m_min == label_info.m_cardinality.m_max)
                    {
                        /* non-variadic argument */

                        const Tensor& label = labels[label_index];
                        const Tensor& target = targets[target_index];

                        if (target_index >= labels.size())
                            return;

                        if (IsConstant(label))
                        {
                            if (!AlwaysEqual(label, target))
                                return;
                        }
                        else if (NameIs(label, builtin_names::Identifier))
                        {
                            // check type
                            if (Is(target, label))
                            {
                                candidate->AddSubstitution(GetIdentifierName(label), target);
                            }
                        }
                        else
                        {
                            // function call
                            if (label.GetExpression()->GetName() == target.GetExpression()->GetName())
                            {
                                DiscrNodeToProcess node_to_process;
                                node_to_process.m_targets = target.GetExpression()->GetArguments();
                                node_to_process.m_source_discr_node = edge.m_dest_node;
                                node_to_process.m_parent_candidate_handle = i_candidate_handle;
                                m_discr_node_stack.push_back(node_to_process);
                            }
                        }

                    }
                    else
                    {
                                    /* variadic argument */

                        UInt label_size = NumericCast<UInt>(labels.size());
                        UInt target_size = NumericCast<UInt>(targets.size());

                        // number of total parameters usable for the repeated pattern
                        Range usable;
                        
                        assert(target_size >= label_infos[label_index].m_remaining.m_min + label_index);
                        usable.m_max = target_size - label_infos[label_index].m_remaining.m_min - label_index;
                        if (label_infos[label_index].m_remaining.m_max == Range::s_infinite)
                            usable.m_min = 0;
                        else
                        {
                            assert(target_size >= label_infos[label_index].m_remaining.m_max + label_index);
                            usable.m_min = target_size - label_infos[label_index].m_remaining.m_max - label_index;
                        }

                        usable = label_infos[label_index].m_cardinality.ClampRange(usable);
                        
                        // align the usable range to be a multiple of sub_pattern_count
                        const UInt sub_pattern_count = NumericCast<UInt>( 
                            labels[label_index].GetExpression()->GetArguments().size() );
                        usable.m_min += sub_pattern_count - 1;
                        usable.m_min -= usable.m_min % sub_pattern_count;
                        usable.m_max -= usable.m_max % sub_pattern_count;

                        SubstGraphDebugPrintLn("\tadding variadic to use (",
                            usable, " terms)");

                        // used: total number of targets used in the target
                        // rep: number of times the repetition is repeated
                        uint32_t rep = NumericCast<uint32_t>(usable.m_min / sub_pattern_count);
                        for (size_t used = usable.m_min; used <= usable.m_max; 
                            used += sub_pattern_count, rep++)
                        {
                            // repetition candidate
                            CandHandle rep_cand_handle = m_candidates.New();
                            Candidate& rep_candidate = m_candidates.GetObject(rep_cand_handle);
                            rep_candidate.m_targets = targets.subspan(target_index, used);
                            rep_candidate.m_edge = &edge;
                            rep_candidate.m_label_offset = label_index;
                            rep_candidate.m_parent_candidate = parent_candidate_handle;
                            rep_candidate.m_source_discr_node = source_discr_node;
                            rep_candidate.m_repetitions = rep;
                            m_pending_candidates.push_back(rep_cand_handle);

                            // continuation candidate
                            CandHandle cont_cand_handle = m_candidates.New();
                            Candidate& cont_candidate = m_candidates.GetObject(cont_cand_handle);
                            cont_candidate.m_targets = targets.subspan(target_index + used);
                            cont_candidate.m_edge = &edge;
                            cont_candidate.m_label_offset = label_index + used;
                            cont_candidate.m_parent_candidate = rep_cand_handle;
                            m_pending_candidates.push_back(cont_cand_handle);

                            candidate = &m_candidates.GetObject(i_candidate_handle);
                        }
                        return;
                    }
                }                
            }

            // check if the discrimination edge is complete
            bool labels_finished = true; /*edge.m_pattern_info.m_labels_info.data() +
                edge.m_pattern_info.m_labels_info.size() == &label_infos[label_index]; */

            std::vector<Substitution> & substitutions = m_solution_tree[discr_dest_node].m_substitutions;



            /*SolutionTreeItem & solution_item = m_solution_tree[edge.m_dest_node];
            for(Substitution & subst : substitutions)
                m_solution_tree[edge.m_dest_node].m_substitutions.push_back(subst);*/
        }

    } // namespace pattern

} // namespace djup
