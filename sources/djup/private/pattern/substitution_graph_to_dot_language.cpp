
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/pattern/substitution_graph.h>
#include <private/pattern/discrimination_net.h>
#include <private/pattern/candidate.h>

namespace djup
{
    namespace pattern
    {
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
