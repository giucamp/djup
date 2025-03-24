
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
            std::vector<Span<const Tensor>> Tokenize(const Tensor & i_tensor)
            {
                std::vector<Span<const Tensor>> result;
                result.push_back({ &i_tensor, 1 });

                for (size_t i = 0; i < result.size(); i++)
                {
                    Span<const Tensor> arguments = result[i];

                    for (size_t j = 0; j < arguments.size(); j++)
                    {
                        if (!IsLiteral(arguments[j]) && !IsIdentifier(arguments[j]) && !IsRepetition(arguments[j]))
                        {
                            result.push_back(Span(arguments[j].GetExpression()->GetArguments()));
                        }
                    }
                }

                return result;
            }

            std::string TensorSpanToString(Span<const Tensor> i_tensor)
            {
                std::string result;
                for (size_t i = 0; i < i_tensor.size(); i++)
                {
                    if (i != 0)
                        result += ", ";
                    result += ToSimplifiedStringForm(i_tensor[i]);
                }
                return result;
            }
        }

        SubstitutionGraph::SubstitutionGraph(const DiscriminationNet & m_discrimination_net)
            : m_discrimination_net(m_discrimination_net)
        {

        }

        SubstitutionGraph::~SubstitutionGraph() = default;

        struct SubstitutionGraph::Node
        {
            uint32_t m_outgoing_edges{};
        };

        struct SubstitutionGraph::Substitution
        {
            Name m_variable_name;
            Tensor m_value;
        };

        struct SubstitutionGraph::CandidateData
        {
            uint16_t m_pattern_offset{};
            uint16_t m_target_offset{};

            Span<const Tensor> m_targets;
            
            uint32_t m_discrimination_node{ std::numeric_limits<uint32_t>::max() };
            const DiscriminationNet::Edge* m_discrimination_edge{nullptr};

            uint32_t m_repetitions_offset{};
            uint32_t m_repetitions{ std::numeric_limits<uint32_t>::max() };

            uint32_t m_open{};
            uint32_t m_close{};

            std::vector<Substitution> m_substitutions;

            bool AddSubstitution(const Name& i_variable_name, const Tensor& i_value)
            {
                m_substitutions.emplace_back(Substitution{ i_variable_name, i_value });
                return true;
            }
        };

        struct SubstitutionGraph::Candidate
        {
            uint32_t m_start_node{};
            uint32_t m_end_node{};
            
            CandidateData m_data;

            uint32_t m_version{};
            bool m_decayed = false;
        };

        void SubstitutionGraph::AddCandidate(uint32_t i_start_node, uint32_t i_end_node, 
            CandidateData i_new_candidate_data, std::vector<Substitution>&& i_substitutions)
        {
            assert(i_start_node != i_end_node);

            Candidate candidate;
            candidate.m_data = i_new_candidate_data;
            candidate.m_version = m_next_candidate_version++;
            candidate.m_start_node = i_start_node;
            candidate.m_end_node = i_end_node;

            const uint32_t new_candidate_index = NumericCast<uint32_t>(m_candidate_stack.size());
            CandidateRef candidate_ref{ new_candidate_index, candidate.m_version };

            AddEdge(i_start_node, i_end_node, candidate_ref, 
                i_new_candidate_data.m_open, i_new_candidate_data.m_close,
                std::move(i_substitutions));

            m_candidate_stack.push_back(std::move(candidate));
        }

        bool SubstitutionGraph::IsCandidateRefValid(CandidateRef i_ref) const
        {
            return i_ref.m_index < m_candidate_stack.size() && i_ref.m_version == m_candidate_stack[i_ref.m_index].m_version;
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
                        Candidate& candidate = m_candidate_stack[it->second.m_candidate_ref.m_index];
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

        void SubstitutionGraph::AddEdge(uint32_t i_start_node, uint32_t i_end_node,
            CandidateRef i_candidate_ref, uint32_t i_open, uint32_t i_close,
            std::vector<Substitution>&& i_substitutions)
        {
            m_edges.insert({ i_end_node, Edge{i_start_node, i_candidate_ref, i_open, i_close, std::move(i_substitutions)}});
            m_nodes[i_start_node].m_outgoing_edges++;
        }

        void SubstitutionGraph::RemoveEdge(uint32_t i_start_node, uint32_t i_dest_node, CandidateRef i_candidate_ref)
        {
            bool found = false;
            const auto range = m_edges.equal_range(i_dest_node);
            for (auto it = range.first; it != range.second; it++)
            {
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

        bool SubstitutionGraph::MatchDiscriminationEdge(
            const Candidate & i_candidate, const DiscriminationNet::Edge & i_discrimination_edge)
        {
            std::vector<Substitution> substitutions;

            const bool nest_index = i_candidate.m_data.m_repetitions != std::numeric_limits<uint32_t>::max();
            const uint32_t repetitions = nest_index ? i_candidate.m_data.m_repetitions : 1;

            // const Span<const Tensor> token = m_tokens[i_candidate.m_data.m_token_index];

            const Span<const Tensor> targets = i_candidate.m_data.m_targets; /*i_candidate.m_data.m_target_size == std::numeric_limits<uint16_t>::max() ?
                token.subspan(i_candidate.m_data.m_target_offset) :
                token.subspan(i_candidate.m_data.m_target_offset, i_candidate.m_data.m_target_size);*/
            
            if (!i_discrimination_edge.m_cardinality.IsWithin(targets.size()))
                return false;

            size_t target_index = i_candidate.m_data.m_target_offset;
            for (uint32_t repetition = i_candidate.m_data.m_repetitions_offset; repetition < repetitions; repetition++)
            {
                const size_t pattern_size = i_discrimination_edge.m_patterns.size();
                for (size_t pattern_index = i_candidate.m_data.m_pattern_offset;
                    pattern_index < pattern_size; target_index++, pattern_index++)
                {   
                    const Tensor & pattern = i_discrimination_edge.m_patterns[pattern_index];

                    const Range argument_cardinality = i_discrimination_edge.m_argument_infos[pattern_index].m_cardinality;
                    const Range remaining = i_discrimination_edge.m_argument_infos[pattern_index].m_remaining;
                    const Range cardinality = i_discrimination_edge.m_cardinality;

                    if (argument_cardinality.m_min != argument_cardinality.m_max)
                    {
                        size_t total_available_targets = targets.size() - target_index;

                        size_t sub_pattern_count = pattern.GetExpression()->GetArguments().size();
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

                        uint32_t rep = NumericCast<uint32_t>(usable.m_min / sub_pattern_count);
                        for (size_t used = usable.m_min; used <= usable.m_max; used += sub_pattern_count, rep++)
                        {
                            assert(!nest_index); // repetitions can't be nested directly

                            const uint32_t meddle_node = NewNode();

                            CandidateData candidates[2];

                            // repeated pattern
                            candidates[0].m_open = i_candidate.m_data.m_open + 1;
                            candidates[0].m_close = 1;
                            candidates[0].m_repetitions = rep;
                            candidates[0].m_targets = targets.subspan(target_index, used);
                            candidates[0].m_discrimination_node = i_discrimination_edge.m_dest_node;                            
                            AddCandidate(i_candidate.m_start_node, meddle_node, candidates[0], std::move(substitutions)); 
                            
                            // rest of pattern
                            candidates[1].m_close = i_candidate.m_data.m_close;
                            candidates[1].m_pattern_offset = pattern_index + 1;
                            candidates[1].m_targets = targets;
                            candidates[1].m_target_offset = target_index + used;
                            candidates[1].m_discrimination_node = i_candidate.m_data.m_discrimination_node;
                            candidates[1].m_discrimination_edge = &i_discrimination_edge;
                            AddCandidate(meddle_node, i_candidate.m_end_node, candidates[1], {});

                            /*LinearPath path(i_context, i_candidate);

                            // pre-pattern
                            path.AddEdge(i_candidate.m_target_arguments.subspan(target_index, used),
                                PatternSegment{ pattern_info.m_flags,
                                    pattern.GetExpression()->GetArguments(),
                                    pattern_info.m_arguments },
                                true, rep);

                            // post-pattern
                            path.AddEdge(i_candidate.m_target_arguments.subspan(target_index + used),
                                PatternSegment{ pattern_info.m_flags,
                                i_candidate.m_pattern.m_pattern.subspan(pattern_index + 1),
                                i_candidate.m_pattern.m_arguments.subspan(pattern_index + 1) });*/
                        }
                    }
                    else
                    {
                        const Tensor& target = targets[target_index];

                        if (IsConstant(pattern))
                        {
                            if (!AlwaysEqual(pattern, target))
                                return false;
                        }
                        else if (NameIs(pattern, builtin_names::Identifier))
                        {
                            if (!Is(target, pattern))
                                return false; // type mismatch

                            substitutions.push_back({GetIdentifierName(pattern), target});
                        }
                        else
                        {
                            if (pattern.GetExpression()->GetName() != target.GetExpression()->GetName())
                                return false;

                            CandidateData args_candidate;
                            args_candidate.m_discrimination_node = i_discrimination_edge.m_dest_node;
                            args_candidate.m_discrimination_edge = nullptr;
                            args_candidate.m_open = i_candidate.m_data.m_open;
                            args_candidate.m_targets = target.GetExpression()->GetArguments();

                            CandidateData continuation_candidate = i_candidate.m_data;
                            continuation_candidate.m_discrimination_edge = &i_discrimination_edge;
                            continuation_candidate.m_pattern_offset = pattern_index + 1;
                            continuation_candidate.m_target_offset = target_index + 1;
                            continuation_candidate.m_repetitions_offset = repetition;
                            continuation_candidate.m_open = 0;
                            assert(continuation_candidate.m_pattern_offset <= continuation_candidate.m_discrimination_edge->m_patterns.size());

                            const uint32_t middle_node = NewNode();
                            AddCandidate(i_candidate.m_start_node, middle_node, continuation_candidate, std::move(substitutions));
                            AddCandidate(middle_node, i_candidate.m_end_node, args_candidate, {});

                            return true;
                        }
                    }
                }
            }

            AddEdge(i_candidate.m_start_node, i_candidate.m_end_node, {}, 
                i_candidate.m_data.m_open, i_candidate.m_data.m_close,
                std::move(substitutions));

            if (i_discrimination_edge.m_is_leaf)
            {
                m_terminal_nodes.push_back({ i_candidate.m_end_node , i_discrimination_edge.m_pattern_id });
            }

            return true;
        }

        void SubstitutionGraph::MatchCandidate(
            Candidate && i_candidate, std::function<void()> i_step_callback)
        {
            const uint32_t discrimination_node = i_candidate.m_data.m_discrimination_node;
            assert(discrimination_node != std::numeric_limits<uint32_t>::max());

            // MatchCandidate may add other candidates, take the index before
            const uint32_t candidate_index = NumericCast<uint32_t>(m_candidate_stack.size());
            
            if (i_candidate.m_data.m_discrimination_edge != nullptr)
            {
                MatchDiscriminationEdge(i_candidate, *i_candidate.m_data.m_discrimination_edge);
            }
            else
            {
                for (const auto & edge_it : m_discrimination_net.EdgesFrom(discrimination_node))
                {
                    MatchDiscriminationEdge(i_candidate, edge_it.second);
                }
            }

            RemoveEdge(i_candidate.m_start_node, i_candidate.m_end_node, { candidate_index, i_candidate.m_version });

            if (i_step_callback)
                i_step_callback();
        }

        void SubstitutionGraph::FindMatches(const Tensor & i_target, std::function<void()> i_step_callback)
        {
            m_nodes.resize(2);
            m_nodes[s_start_node_index].m_outgoing_edges++;

            CandidateData root_candidate;
            root_candidate.m_discrimination_node = m_discrimination_net.GetStartNode();
            root_candidate.m_targets = {&i_target, 1};
            AddCandidate(s_start_node_index, s_end_node_index, std::move(root_candidate), {});
            
            if (i_step_callback)
                i_step_callback();

            do {
            
                Candidate candidate = std::move(m_candidate_stack.back());
                m_candidate_stack.pop_back();

                if(!candidate.m_decayed)
                {
                    MatchCandidate(std::move(candidate), i_step_callback);
                }

            } while(!m_candidate_stack.empty());
        }

        uint32_t SubstitutionGraph::NewNode()
        {
            const uint32_t new_node = NumericCast<uint32_t>(m_nodes.size());
            m_nodes.emplace_back();
            return new_node;
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

            const std::string_view escaped_newline = "\\n";

            // next_candidate
            const Candidate* next_candidate = nullptr;
            for (auto it = m_candidate_stack.rbegin(); it != m_candidate_stack.rend(); it++)
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
                    const Candidate candidate = m_candidate_stack[edge.second.m_candidate_ref.m_index];
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
                    const Candidate& candidate = m_candidate_stack[edge.second.m_candidate_ref.m_index];

                    std::string label;
                    const Span<const Tensor> targets = candidate.m_data.m_targets;
                    
                    label = TensorSpanToString(targets.subspan(candidate.m_data.m_target_offset));
                    label += "\\nis\\n";

                    if (candidate.m_data.m_discrimination_edge != nullptr)
                    {
                        label += TensorSpanToString(Span(candidate.m_data.m_discrimination_edge->m_patterns).subspan(candidate.m_data.m_pattern_offset));
                        label += "\\n";
                    }
                    else
                    {
                        for (auto edge_it : m_discrimination_net.EdgesFrom(candidate.m_data.m_discrimination_node))
                        {
                            label += TensorSpanToString(Span(edge_it.second.m_patterns).subspan(candidate.m_data.m_pattern_offset));
                            label += "\\n";
                        }
                    }
                    
                    if (candidate.m_data.m_repetitions != std::numeric_limits<uint32_t>::max())
                        label += ToString(" (", candidate.m_data.m_repetitions, " times)\\n");
                    
                    label += ToString("d: ", candidate.m_data.m_discrimination_node);

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
