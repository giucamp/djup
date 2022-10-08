
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/pattern/substitution_graph.h>
#include <private/pattern/discrimination_net.h>
#include <private/substitute_by_predicate.h>
#include <private/builtin_names.h>
#include <core/flags.h>
#include <core/to_string.h>

namespace djup
{
    namespace pattern
    {
        namespace
        {
            std::string TensorListToString(Span<const Tensor> i_tensors)
            {
                std::string result;
                for(size_t i = 0; i < i_tensors.size(); i++)
                {
                    if(i)
                        result += ", ";
                    result += ToSimplifiedStringForm(i_tensors[i]);
                }
                return result;
            }

            Tensor PreprocessPattern(const Tensor & i_pattern)
            {
                return SubstituteByPredicate(i_pattern, [](const Tensor & i_candidate){
                    FunctionFlags flags = GetFunctionFlags(i_candidate.GetExpression()->GetName());

                    bool some_substitution = false;
                    std::vector<Tensor> new_arguments;

                    const std::vector<Tensor> & arguments = i_candidate.GetExpression()->GetArguments();
                    const size_t argument_count = arguments.size();

                    // substitute identifiers in associative functions with AssociativeIdentifier()
                    if(HasFlag(flags, FunctionFlags::Associative))
                    {
                        size_t index = 0;

                        for(; index < argument_count; index++)
                        {
                            const Tensor & argument = arguments[index];
                            if(IsIdentifier(argument))
                            {
                                new_arguments = arguments;
                                some_substitution = true;
                                break;
                            }
                        }

                        for(; index < argument_count; index++)
                        {
                            const Tensor & argument = arguments[index];
                            if(IsIdentifier(argument))
                            {
                                new_arguments[index] = MakeExpression(builtin_names::AssociativeIdentifier, 
                                    {argument}, 
                                    argument.GetExpression()->GetMetadata());
                            }
                        }
                    }

                    if(some_substitution)
                        return MakeExpression(i_candidate.GetExpression()->GetName(), new_arguments, i_candidate.GetExpression()->GetMetadata());
                    else
                        return i_candidate;
                });
            }
        }

        struct SubstitutionGraph::Node
        {
            std::string m_debug_name;
            size_t m_outgoing_edges{};
        };

        struct SubstitutionGraph::Substitution
        {
            Name m_variable_name;
            Tensor m_value;
        };

        struct SubstitutionGraph::Candidate
        {
            uint32_t m_start_node{};
            Span<const Tensor> m_targets;


            uint32_t m_repetitions{};
            uint32_t m_version{};
            bool m_decayed = false;
            uint32_t m_open{};
            uint32_t m_close{};
            std::vector<Substitution> m_substitutions;
        };

        struct SubstitutionGraph::CandidateRef
        {
            uint32_t m_index = std::numeric_limits<uint32_t>::max();
            uint32_t m_version{};
        };

        struct SubstitutionGraph::Edge
        {
            uint32_t m_source_index{};
            CandidateRef m_candidate_ref;
            std::vector<Substitution> m_substitutions;
            uint32_t m_open{};
            uint32_t m_close{};
        };

        void SubstitutionGraph::AddEdge(uint32_t i_source_node, uint32_t i_dest_node, 
            std::vector<Substitution> && i_substitutions)
        {
            Edge edge;
            edge.m_source_index = i_source_node;
            edge.m_substitutions = std::move(i_substitutions);
            m_edges.insert(std::pair(i_dest_node, std::move(edge)));
        }

        #if 1

        bool SubstitutionGraph::MatchCandidate(const DiscriminationNet & i_discrimination_net, Candidate & i_candidate)
        {
            for(auto edge_it : i_discrimination_net.EdgesFrom(i_candidate.m_start_node))
            {
                const Tensor & target = i_candidate.m_targets.front();

                const Tensor & pattern = edge_it.second.m_expression;
                const Range cardinality = edge_it.second.m_info.m_cardinality;
                const Range remaining = edge_it.second.m_info.m_remaining;
                const Range argument_cardinality = edge_it.second.m_argument_cardinality;

                if(cardinality.m_min != cardinality.m_max)
                {
                    size_t sub_pattern_count = pattern.GetExpression()->GetArguments().size();
                    assert(sub_pattern_count != 0); // empty repetitions are illegal and should raise an error when constructed

                    // compute usable range
                    Range usable;
                    size_t available_targets = i_candidate.m_targets.size();
                    usable.m_max = available_targets - remaining.m_min;
                    usable.m_min = remaining.m_max == Range::s_infinite ?
                        0 : available_targets - remaining.m_max;
                    usable = cardinality.Clamp(usable);

                    // align the usable range to be a multiple of sub_pattern_count
                    usable.m_min += sub_pattern_count - 1;
                    usable.m_min -= usable.m_min % sub_pattern_count;
                    usable.m_max -= usable.m_max % sub_pattern_count;

                    uint32_t rep = NumericCast<uint32_t>(usable.m_min / sub_pattern_count);
                    for(size_t used = usable.m_min; used <= usable.m_max; used += sub_pattern_count, rep++)
                    {
                        /*LinearPath path(i_context, i_candidate);

                        // pre-pattern
                        path.AddEdge(i_candidate.m_targets.subspan(0, used),
                            PatternSegment{ pattern_info.m_flags,
                            pattern.GetExpression()->GetArguments(),
                            pattern_info.m_arguments},
                            true, rep );

                        // post-pattern
                        path.AddEdge(i_candidate.m_targets.subspan(0+ used),
                            PatternSegment{ pattern_info.m_flags,
                            i_candidate.m_pattern.m_pattern.subspan(pattern_index + 1),
                            i_candidate.m_pattern.m_arguments.subspan(pattern_index + 1) } );*/
                    }
                    return false;
                }
                
                if(IsConstant(pattern))
                {
                    if(!AlwaysEqual(pattern, target))
                        return false;
                }
                else if(NameIs(pattern, builtin_names::Identifier))
                {
                    if(!Is(target, pattern))
                        return false; // type mismatch

                    AddEdge(i_candidate.m_start_node, edge_it.second.m_dest_node, {Substitution{GetIdentifierName(pattern), target}});
                }
                else 
                {
                    if(pattern.GetExpression()->GetName() != target.GetExpression()->GetName())
                        return false;

                    // if the target does not have enough arguments, early reject
                    size_t target_arguments = target.GetExpression()->GetArguments().size();
                    if(target_arguments >= argument_cardinality.m_min &&
                        target_arguments <= argument_cardinality.m_max )
                    {
                        /*LinearPath path(i_context, i_candidate);

                        // match content
                        path.AddEdge(target.GetExpression()->GetArguments(), 
                            PatternSegment{ pattern_info.m_flags,
                            pattern.GetExpression()->GetArguments(),
                            pattern_info.m_arguments });

                        // rest of this repetition
                        const size_t remaining_in_pattern = i_candidate.m_pattern.m_pattern.size() - (pattern_index + 1);
                        path.AddEdge(i_candidate.m_targets.subspan(target_index + 1, remaining_in_pattern), 
                            PatternSegment{ pattern_info.m_flags,
                            i_candidate.m_pattern.m_pattern.subspan(pattern_index + 1),
                            i_candidate.m_pattern.m_arguments.subspan(pattern_index + 1) } );

                        // remaining repetitions
                        const size_t target_start = target_index + 1 + remaining_in_pattern;
                        path.AddEdge(i_candidate.m_targets.subspan(target_start),
                            i_candidate.m_pattern, false, repetitions - (repetition + 1) );*/
                    }
                    return false;
                }
            }
            return false;
        }

        #endif

        void SubstitutionGraph::FindMatches(const DiscriminationNet & i_discrimination_net,
            const Tensor & i_target, const Tensor & i_condition)
        {
            Candidate first_candidate;
            first_candidate.m_start_node = i_discrimination_net.GetStartNode();
            first_candidate.m_targets = {i_target, 1};
            m_candidates.push_back(std::move(first_candidate));

            do {
            
                Candidate candidate = std::move(m_candidates.back());
                m_candidates.pop_back();
                
                // MatchCandidate may add other candidates, take the index before
                const uint32_t candidate_index = NumericCast<uint32_t>(m_candidates.size());

                if(!candidate.m_decayed)
                {
                    const bool match = MatchCandidate(i_discrimination_net, candidate);


                }

            } while(!m_candidates.empty());
        }

    } // namespace pattern

} // namespace djup
