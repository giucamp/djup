
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/pattern/substitution_graph.h>
#include <private/pattern/discrimination_net.h>
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
        }

        struct SubstitutionGraph::Node
        {
            size_t m_outgoing_edges{};
        };

        struct SubstitutionGraph::Substitution
        {
            Name m_variable_name;
            Tensor m_value;
        };

        struct SubstitutionGraph::Candidate
        {
            uint32_t m_discrimination_node{};
            uint32_t m_start_node{};
            uint32_t m_end_node{};
            Span<const Tensor> m_targets;

            uint32_t m_repetitions = std::numeric_limits<uint32_t>::max();
            uint32_t m_version{};
            bool m_decayed = false;
            uint32_t m_open{};
            uint32_t m_close{};
            std::vector<Substitution> m_substitutions;

            bool AddSubstitution(Candidate& i_candidate, const Name& i_variable_name, const Tensor& i_value)
            {
                m_substitutions.emplace_back(Substitution{ i_variable_name, i_value });
                return true;
            }
        };

        bool SubstitutionGraph::IsCandidateRefValid(CandidateRef i_ref) const
        {
            return i_ref.m_index < m_candidates.size() && i_ref.m_version == m_candidates[i_ref.m_index].m_version;
        }

        void SubstitutionGraph::RemoveNode(uint32_t i_node_index)
        {
            std::vector<uint32_t> nodes_to_remove;
            nodes_to_remove.push_back(i_node_index);

            while (!nodes_to_remove.empty())
            {
                uint32_t node = nodes_to_remove.back();
                nodes_to_remove.pop_back();

                const auto range = m_edges.equal_range(node);
                for (auto it = range.first; it != range.second;)
                {
                    uint32_t start_node = it->second.m_source_index;

                    if (IsCandidateRefValid(it->second.m_candidate_ref))
                    {
                        Candidate& candidate = m_candidates[it->second.m_candidate_ref.m_index];
                        if (candidate.m_start_node == it->second.m_source_index &&
                            candidate.m_end_node == it->first)
                        {
                            candidate.m_decayed = true;
                        }
                    }

                    assert(m_nodes[it->second.m_source_index].m_outgoing_edges > 0);
                    m_nodes[it->second.m_source_index].m_outgoing_edges--;
                    it = m_edges.erase(it);

                    if (m_nodes[start_node].m_outgoing_edges == 0)
                    {
                        // we have removed the last outcoming edge for i_start_node, we can erase it
                        nodes_to_remove.push_back(start_node);
                    }
                }
            }
        }

        void SubstitutionGraph::RemoveEdge(uint32_t i_start_node, uint32_t i_dest_node, CandidateRef i_candidate_ref)
        {
            bool found = false;
            const auto range = m_edges.equal_range(i_dest_node);
            for (auto it = range.first; it != range.second; it++)
            {
                // the candidate has just been removed from the stack
                // assert(IsCandidateRefValid(i_context, it->second.m_candidate_ref));

                if (it->second.m_source_index == i_start_node &&
                    it->second.m_candidate_ref.m_index == i_candidate_ref.m_index &&
                    it->second.m_candidate_ref.m_version == i_candidate_ref.m_version)
                {
                    assert(m_nodes[i_start_node].m_outgoing_edges > 0);
                    m_nodes[i_start_node].m_outgoing_edges--;
                    m_edges.erase(it);
                    found = true;
                    break;
                }
            }
            assert(found);

            if (m_nodes[i_start_node].m_outgoing_edges == 0)
            {
                // we have removed the last outcoming edge for i_start_node, we can erase it
                RemoveNode(i_start_node);
            }
        }

        class SubstitutionGraph::LinearPath
        {
        public: 

            LinearPath(const Candidate & i_source_candidate, uint32_t i_discrimination_node)
                : m_discrimination_node(i_discrimination_node),
                  m_start_node(i_source_candidate.m_start_node), 
                  m_end_node(i_source_candidate.m_end_node),
                  m_open(i_source_candidate.m_open),
                  m_close(i_source_candidate.m_close)
            {
                assert(m_start_node != m_end_node);
            }

            LinearPath(const LinearPath &) = delete;
            LinearPath & operator = (const LinearPath &) = delete;

            void AddEdge(Span<const Tensor> i_targets, uint32_t i_discrimination_node,
                bool i_increase_depth = false, uint32_t i_repetitions = std::numeric_limits<uint32_t>::max())
            {
                assert(m_edge_count < s_max_edges);

                if (!i_targets.empty() && i_repetitions != 0)
                {
                    m_edges[m_edge_count].m_targets = i_targets;
                    m_edges[m_edge_count].m_discrimination_node = i_discrimination_node;
                    m_edges[m_edge_count].m_repetitions = i_repetitions;

                    if (i_increase_depth)
                    {
                        m_edges[m_edge_count].m_open = 1;
                        m_edges[m_edge_count].m_close = 1;
                    }

                    ++m_edge_count;
                }
            }

            void Commit(SubstitutionGraph& i_substitution_graph)
            {
                uint32_t start_node = m_start_node;

                assert(m_edge_count > 0);

                uint32_t open[s_max_edges] = {};
                uint32_t close[s_max_edges] = {};

                m_edges[0].m_open += m_open;
                m_edges[m_edge_count - 1].m_close += m_close;

                for (uint32_t i = 0; i < m_edge_count; i++)
                {
                    uint32_t end_node;
                    if (i == m_edge_count - 1)
                    {
                        end_node = m_end_node;
                    }
                    else
                    {
                       const uint32_t intermediate_node = NumericCast<uint32_t>(i_substitution_graph.m_nodes.size());
                        i_substitution_graph.m_nodes.emplace_back();
                        end_node = intermediate_node;
                    }
                    
                    i_substitution_graph.AddCandidate(start_node, end_node, m_edges[i].m_targets,
                        m_edges[i].m_discrimination_node, m_edges[i].m_open, m_edges[i].m_close, m_edges[i].m_repetitions );

                    start_node = end_node;
                }
            }

        private:
;           const uint32_t m_discrimination_node{};
            const uint32_t m_start_node{};
            const uint32_t m_end_node{};
            const uint32_t m_open;
            const uint32_t m_close;

            struct Edge
            {
                Span<const Tensor> m_targets;
                uint32_t m_discrimination_node{};
                uint32_t m_repetitions{};
                uint32_t m_open{}, m_close{};
            };
            static constexpr uint32_t s_max_edges = 3;
            Edge m_edges[s_max_edges];
            uint32_t m_edge_count{};
        };

        bool SubstitutionGraph::MatchCandidate(const DiscriminationNet & i_discrimination_net, Candidate & i_candidate)
        {
            const bool nest_index = i_candidate.m_repetitions != std::numeric_limits<uint32_t>::max();
            const uint32_t repetitions = nest_index ? i_candidate.m_repetitions : 1;

            for (auto edge_it : i_discrimination_net.EdgesFrom(i_candidate.m_discrimination_node))
            {
                const Tensor& pattern = edge_it.second.m_expression;
                const Range cardinality = edge_it.second.m_info.m_cardinality;
                const Range remaining = edge_it.second.m_info.m_remaining;
                const Range argument_cardinality = edge_it.second.m_argument_cardinality;

                size_t target_index = 0;
                for (uint32_t repetition = 0; repetition < repetitions; repetition++)
                {

                }
            }


#if 0


            for(auto edge_it : i_discrimination_net.EdgesFrom(i_candidate.m_discrimination_node))
            {
                const Tensor & target = i_candidate.m_targets.front();

                const Tensor & pattern = edge_it.second.m_expression;
                const Range cardinality = edge_it.second.m_info.m_cardinality;
                const Range remaining = edge_it.second.m_info.m_remaining;
                const Range argument_cardinality = edge_it.second.m_argument_cardinality;

                if(cardinality.m_min != cardinality.m_max)
                {
                    uint32_t sub_pattern_count = NumericCast<uint32_t>(pattern.GetExpression()->GetArguments().size());
                    assert(sub_pattern_count != 0); // empty repetitions are illegal and should raise an error when constructed

                    // compute usable range
                    Range usable;
                    uint32_t available_targets = NumericCast<uint32_t>(i_candidate.m_targets.size());
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
                        LinearPath path(i_candidate, edge_it.second.m_dest_node);

                        // pre-pattern
                        path.AddEdge(LinearPath::First, i_candidate.m_targets.subspan(0, used));

                        // post-pattern
                        path.AddEdge(LinearPath::Last, i_candidate.m_targets.subspan(used));

                        // pre-pattern
                        /*path.AddEdge(i_candidate.m_targets.subspan(0, used),
                            PatternSegment{ pattern_info.m_flags,
                            pattern.GetExpression()->GetArguments(),
                            pattern_info.m_arguments},
                            true, rep );

                        // post-pattern
                        path.AddEdge(i_candidate.m_targets.subspan(0+ used),
                            PatternSegment{ pattern_info.m_flags,
                            i_candidate.m_pattern.m_pattern.subspan(pattern_index + 1),
                            i_candidate.m_pattern.m_arguments.subspan(pattern_index + 1) } );*/

                        path.Commit(*this);
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
                        LinearPath path(*this, i_candidate, edge_it.second.m_dest_node);

                        // match content
                        /*path.AddEdge(target.GetExpression()->GetArguments(), 
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

#endif
            return false;
        }

        void SubstitutionGraph::AddCandidate(
            uint32_t i_start_node, uint32_t i_end_node,
            Span<const Tensor> i_targets, uint32_t i_discrimination_node,
            uint32_t i_open, uint32_t i_close, uint32_t i_repetitions)
        {
            assert(i_start_node != i_end_node);

            Candidate new_candidate;
            new_candidate.m_start_node = i_start_node;
            new_candidate.m_end_node = i_end_node;
            new_candidate.m_discrimination_node = i_discrimination_node;
            new_candidate.m_targets = i_targets;
            new_candidate.m_repetitions = i_repetitions;
            new_candidate.m_version = m_next_candidate_version++;
            new_candidate.m_open = i_open;
            new_candidate.m_close = i_close;

            const uint32_t new_candidate_index = NumericCast<uint32_t>(m_candidates.size());
            CandidateRef candidate_ref{ new_candidate_index, new_candidate.m_version };
            m_edges.insert({ i_end_node, Edge{i_start_node, candidate_ref, {}, i_open, i_close } });
            m_nodes[i_start_node].m_outgoing_edges++;

            m_candidates.push_back(std::move(new_candidate));
        }

        void SubstitutionGraph::FindMatches(const DiscriminationNet & i_discrimination_net,
            const Tensor & i_target, const Tensor & i_condition)
        {
            Candidate first_candidate;
            first_candidate.m_discrimination_node = i_discrimination_net.GetStartNode();
            first_candidate.m_start_node = s_start_node_index;
            first_candidate.m_end_node = s_end_node_index;
            first_candidate.m_targets = {i_target, 1};
            m_candidates.push_back(std::move(first_candidate));

            m_nodes.resize(2);
            
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

        std::string SubstitutionGraph::ToDotLanguage(std::string_view i_graph_name) const
        {
            StringBuilder dest;

            dest << "digraph G";
            dest.NewLine();
            dest << "{";
            dest.NewLine();
            dest.Tab();

            dest << "label = \"" << i_graph_name << "\"";
            dest.NewLine();

            const std::string escaped_newline = "\\n";

            // next_candidate
            const Candidate* next_candidate = nullptr;
            for (auto it = m_candidates.rbegin(); it != m_candidates.rend(); it++)
            {
                if (!it->m_decayed)
                {
                    next_candidate = &*it;
                    break;
                }
            }

            for (size_t i = 0; i < m_nodes.size(); i++)
            {
                const Node & node = m_nodes[i];

                dest << "v" << i << "[label = \"";
                if (i == 0)
                    dest << "Final" << escaped_newline;
                else if (i == 1)
                    dest << "Initial" << escaped_newline;
                else
                    dest << "Node " << i << escaped_newline;

                dest << "\"]";
                dest.NewLine();
            }

            for (const auto& edge : m_edges)
            {
                std::string tail;
                auto append_to_tail = [&](const std::string& i_str) {
                    if (!tail.empty())
                        tail += escaped_newline;
                    tail += i_str;
                };

                /*if (IsCandidateRefValid(edge.second.m_candidate_ref))
                {
                    const Candidate candidate = m_candidates[edge.second.m_candidate_ref.m_index];
                    if (HasAllFlags(candidate.m_pattern.m_flags, CombineFlags(FunctionFlags::Associative, FunctionFlags::Commutative)))
                        append_to_tail("ac");
                    else if (HasFlag(candidate.m_pattern.m_flags, FunctionFlags::Associative))
                        append_to_tail("a");
                    else if (HasFlag(candidate.m_pattern.m_flags, FunctionFlags::Commutative))
                        append_to_tail("a");
                }*/

                if (edge.second.m_open)
                    append_to_tail(" " + std::string(edge.second.m_open, '+') + " ");

                std::string labels;
                if (!tail.empty())
                    labels += " taillabel = \" " + tail + " \" ";
                if (edge.second.m_close)
                    labels += " headlabel = \" " + std::string(edge.second.m_close, '-') + " \" ";

                if (IsCandidateRefValid(edge.second.m_candidate_ref))
                {
                    const Candidate& candidate = m_candidates[edge.second.m_candidate_ref.m_index];

                    std::string label;
                    if (!candidate.m_targets.empty())
                    {
                        label = TensorListToString(candidate.m_targets);
                        if (candidate.m_repetitions != std::numeric_limits<uint32_t>::max())
                            label += ToString(" (", candidate.m_repetitions, " times)");
                    }

                    std::string color = "black";
                    if (&candidate == next_candidate)
                        color = "green4";

                    dest << 'v' << edge.second.m_source_index << " -> v" << edge.first
                        << "[style=\"dashed\", color=\"" << color << "\", label=\"" << label << "\"" << labels << "]" << ';';
                }
                else
                {
                    std::string label;
                    for (const auto& substitution : edge.second.m_substitutions)
                    {
                        label += ToString(substitution.m_variable_name, " = ", ToSimplifiedStringForm(substitution.m_value), escaped_newline);
                    }
                    dest << 'v' << edge.second.m_source_index << " -> v" << edge.first
                        << "[label=\"" << label << "\"" << labels << "]" << ';';
                }
                dest.NewLine();
            }

            dest.Untab();
            dest << "}";
            dest.NewLine();

            return dest.StealString();
        }

    } // namespace pattern

} // namespace djup
