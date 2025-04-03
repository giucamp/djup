
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/pattern/substitution_graph.h>
#include <private/pattern/discrimination_tree.h>
#include <private/pattern/candidate.h>
#include <private/builtin_names.h>
#include <core/flags.h>
#include <core/to_string.h>
#include <private/pattern/utils.h>
#include <assert.h>

namespace djup
{
    namespace pattern
    {
        
        SubstitutionGraph::SubstitutionGraph(const DiscriminationTree& m_discrimination_net)
            : m_discrimination_net(m_discrimination_net)
        {
            m_discr_node_queue.push_back(0);
            
            // this vector maps discrimination nodes to substitution nodes
            m_discr_node_to_substitution_node.resize(m_discrimination_net.GetNodeCount());
        }

        SubstitutionGraph::~SubstitutionGraph() = default;

        /** returns the next candidate index in the chain */
        int32_t SubstitutionGraph::AddCandidate(Candidate&& i_candidate, const char * phase)
        {
            PrintLn("* New Candidate * ", phase);
            PrintLn("\tTarget: ", TensorSpanToString(i_candidate.m_targets.subspan(i_candidate.m_target_offset)) );
            PrintLn("\tPattern: ", TensorSpanToString(
                Span(i_candidate.m_discr_edge.m_arguments).subspan(i_candidate.m_pattern_offset)));
            PrintLn();

            if (!i_candidate.m_discr_edge.m_arguments.empty())
            {
                m_candidate_stack.push_back(std::move(i_candidate));
            }
            return NumericCast<int32_t>(m_candidate_stack.size());
        }

        void SubstitutionGraph::FindMatches(const Tensor& i_target, std::function<void()> i_step_callback)
        {
            ProcessDiscriminationNodes(Span<const Tensor>(&i_target, 1), i_step_callback);

            for (const Solution& solution : m_solutions)
            {
                PrintLn();
                PrintLn("\t**** Solutions ****");
                for (const Substitution& substitution : solution.m_substituitions)
                {
                    PrintLn(substitution.m_variable_name, " = ", ToSimplifiedStringForm(substitution.m_value));
                }
            }
        }

        bool SubstitutionGraph::ProcessDiscriminationNodes(
            Span<const Tensor> i_targets, std::function<void()> i_step_callback)
        {
            PrintLn("----- ProcessDiscriminationNodes, target: ", TensorSpanToString(i_targets));

            int dbg_loop = 0;

            if (i_step_callback)
                i_step_callback();

            do {

                // peek the last discrimination node from the root
                const uint32_t discr_node = m_discr_node_queue.back();
                m_discr_node_queue.pop_back();

                Candidate candidate;
                candidate.m_parent_candidate = m_discr_node_to_substitution_node[discr_node];
                candidate.m_targets = i_targets;

                // tries a match for each outgoing edge from this node
                for (auto& edge_it : m_discrimination_net.EdgesFrom(discr_node))
                {
                    PrintLn();
                    PrintLn("-- Loop ", dbg_loop, ", queue: " );
                    PrintIntVector(m_discr_node_queue);
                    PrintLn();

                    const DiscriminationTree::Edge& edge = edge_it.second;
                    candidate.m_discr_edge = edge;
                    int32_t next_candidate_index = MatchCandidate(
                        m_discr_node_to_substitution_node[discr_node], candidate);

                    if (next_candidate_index != -1)
                    {
                        // target is matching (so far), then add the edge destination to the queue
                        m_discr_node_queue.push_back(edge.m_dest_node);

                        // this will tell the candidate from which discrimination node it should start
                        m_discr_node_to_substitution_node[next_candidate_index] = edge.m_dest_node;

                        // check if the discrimination node is a leaf (solution)
                        /*if (edge.is_leaf_node)
                        {
                            std::vector<Substitution> substitutions;
                            int32_t candidate_index = next_candidate_index;
                            do {
                                substitutions.insert(substitutions.end(),
                                    candidate.m_substitutions.begin(),
                                    candidate.m_substitutions.end());
                                next_candidate_index = m_candidate_stack[next_candidate_index].m_parent_candidate;
                            } while (candidate_index == 0);

                            m_solutions.push_back({ std::move(substitutions) });
                        }*/
                    }

                    dbg_loop++;

                    if (i_step_callback)
                        i_step_callback();
                }

            } while (!m_discr_node_queue.empty());
            return true;
        }
        
        int32_t SubstitutionGraph::MatchCandidate(
            int32_t i_parent_candidate, Candidate& i_candidate)
        {
            // MatchCandidate may add other candidates, take the index before
            const int32_t candidate_index = NumericCast<int32_t>(m_candidate_stack.size());

            /* a candidate may require to the target to have the same expression multiple times,
                (in the case of variadic arguments), otherwise the number of repetitions must be 1 */
            const int32_t repetitions = i_candidate.m_repetitions;

            /* early reject if the number of parameters (targets) is incompatible
               with the number of arguments in the pattern */
            const Span<const Tensor> targets = i_candidate.m_targets;
            if (!i_candidate.m_discr_edge.m_cardinality.IsValaueWithin(
                NumericCast<int32_t>(targets.size())))
            {
                return -1;
            }

            /* loops the repetitions, advancing the target (or argument) at each iteration. */
            int32_t target_index = i_candidate.m_target_offset;
            for (int32_t repetition = i_candidate.m_repetitions_offset; repetition < repetitions; repetition++)
            {
                /** for every repetition the target must have the all the expresssions in
                    the pattern. */
                const int32_t pattern_size = NumericCast<int32_t>(i_candidate.m_discr_edge.m_arguments.size());
                for (int32_t pattern_index = i_candidate.m_pattern_offset;
                    pattern_index < pattern_size; target_index++, pattern_index++)
                {
                    const Tensor& pattern = i_candidate.m_discr_edge.m_arguments[pattern_index];
                    const Range argument_cardinality =
                        i_candidate.m_discr_edge.m_argument_infos[pattern_index].m_cardinality;
                    const Range remaining =
                        i_candidate.m_discr_edge.m_argument_infos[pattern_index].m_remaining;

                    // check if it's a single cardinality argument
                    if (argument_cardinality.m_min == argument_cardinality.m_max)
                    {
                        const Tensor& target = targets[target_index];

                        if (IsConstant(pattern)) /* if the pattern does not have any variable then
                           a full exact match is required (note that expressions are canonicalized) */
                        {
                            if (!AlwaysEqual(pattern, target))
                                return -1;
                        }
                        else if (NameIs(pattern, builtin_names::Identifier))
                        {
                            if (!Is(target, pattern))
                                return -1; // type mismatch

                            i_candidate.m_substitutions.push_back({ GetIdentifierName(pattern), target });
                        }
                        else
                        {
                            // expression call
                            if (pattern.GetExpression()->GetName() != target.GetExpression()->GetName())
                                return false;

                            DiscriminationTree::Edge const& inner_discrimination_node =
                                m_discrimination_net.GetNodeFrom(i_candidate.m_discr_edge.m_dest_node);

                            int32_t next_candidate_index;

                            // first add a candidate with the patterns and arguments of the call
                            Candidate args_candidate = i_candidate;
                            args_candidate.m_parent_candidate = candidate_index;
                            args_candidate.m_discr_edge = inner_discrimination_node;
                            args_candidate.m_open = i_candidate.m_open;
                            args_candidate.m_targets = target.GetExpression()->GetArguments();
                            next_candidate_index = AddCandidate(std::move(args_candidate), "subexpr");

                            // now ass a candidate with the patterns and arguments following the call
                            Candidate continuation_candidate;
                            continuation_candidate.m_parent_candidate = next_candidate_index;
                            //continuation_candidate.m_discr_edge = ;
                            continuation_candidate.m_targets = i_candidate.m_targets;
                            continuation_candidate.m_target_offset = NumericCast<int32_t>(target_index + 1);
                            continuation_candidate.m_pattern_offset = NumericCast<int32_t>(pattern_index + 1);
                            continuation_candidate.m_repetitions_offset = repetition;
                            next_candidate_index = AddCandidate(std::move(continuation_candidate), "remaining");

                            return next_candidate_index;
                        }
                    }
                    else
                    {
                        /* variadic case */
                        int32_t total_available_targets = NumericCast<int32_t>(targets.size()) - target_index;

                        int32_t sub_pattern_count = NumericCast<int32_t>( pattern.GetExpression()->GetArguments().size() );
                        assert(sub_pattern_count != 0); // empty repetitions are illegal and should raise an error when constructed

                        // compute usable range
                        Range usable;
                        usable.m_max = total_available_targets - remaining.m_min;
                        usable.m_min = remaining.m_max == std::numeric_limits<uint32_t>::max() ?
                            0 :
                            total_available_targets - remaining.m_max;

                        usable = argument_cardinality.ClampRange(usable);

                        // align the usable range to be a multiple of sub_pattern_count
                        usable.m_min += sub_pattern_count - 1;
                        usable.m_min -= usable.m_min % sub_pattern_count;
                        usable.m_max -= usable.m_max % sub_pattern_count;

                        // number of possible repetitions
                        uint32_t rep = NumericCast<uint32_t>(usable.m_min / sub_pattern_count);

                        // 'used' is how many arguments we are going to use in this iteration
                        for (size_t used = usable.m_min; used <= usable.m_max; used += sub_pattern_count, rep++)
                        {
                            //assert(!nest_index); // repetitions can't be nested directly

                            /*Candidate repeated_candidate;
                            repeated_candidate.m_parent_candidate = i_parent_candidate;
                            repeated_candidate.m_discr_edge = i_candidate.m_discr_edge;


                                const uint32_t meddle_node = NewNode();

                            Candidate candidates[2];

                            // repeated pattern
                            candidates[0].m_open = i_candidate.m_open + 1;
                            candidates[0].m_close = 1;
                            candidates[0].m_repetitions = rep;
                            candidates[0].m_targets = targets.subspan(target_index, used);
                            candidates[0].m_discrimination_node = i_discrimination_edge.m_dest_node;
                            AddCandidate(i_candidate.m_start_node, meddle_node, candidates[0], std::move(substitutions));

                            // rest of pattern
                            candidates[1].m_close = i_candidate.m_close;
                            candidates[1].m_pattern_offset = pattern_index + 1;
                            candidates[1].m_targets = targets;
                            candidates[1].m_target_offset = target_index + used;
                            candidates[1].m_discrimination_node = i_candidate.m_discrimination_node;
                            candidates[1].m_discrimination_edge = &i_discrimination_edge;
                            AddCandidate(meddle_node, i_candidate.m_end_node, candidates[1], {});

                            return -1;*/
                        }
                    }
                }
            }

            /*AddEdge(i_candidate.m_start_node, i_candidate.m_end_node, {},
                i_candidate.m_open, i_candidate.m_close,
                std::move(substitutions));

            if (i_discrimination_edge.is_leaf_node)
            {
                m_terminal_nodes.push_back({ i_candidate.m_end_node , i_discrimination_edge.m_pattern_id });
            }*/

            return true;
        }
        
        /*{
            if (i_step_callback)
                i_step_callback();

            do {

                Candidate candidate = std::move(m_candidate_stack.back());
                m_candidate_stack.pop_back();

                if (!candidate.m_decayed)
                {
                    MatchCandidate(std::move(candidate), 0);
                }

                if (i_step_callback)
                    i_step_callback();

            } while (!m_candidate_stack.empty());
        }*/




    } // namespace pattern

} // namespace djup
