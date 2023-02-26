
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/pattern/discrimination_net.h>
#include <private/pattern/pattern_info.h>
#include <private/builtin_names.h>
#include <private/expression.h>
#include <private/substitute_by_predicate.h>
#include <core/flags.h>
#include <core/to_string.h>
#include <algorithm> // for min, max

namespace djup
{
    namespace pattern
    {
        std::string TensorSpanToString(Span<const Tensor> i_tensor);

        Tensor PreprocessPattern(const Tensor& i_pattern)
        {
            return SubstituteByPredicate(i_pattern, [](const Tensor& i_candidate) {
                FunctionFlags flags = GetFunctionFlags(i_candidate.GetExpression()->GetName());

                bool some_substitution = false;
                std::vector<Tensor> new_arguments;

                const std::vector<Tensor>& arguments = i_candidate.GetExpression()->GetArguments();
                const size_t argument_count = arguments.size();

                // substitute identifiers in associative functions with AssociativeIdentifier()
                if (HasFlag(flags, FunctionFlags::Associative))
                {
                    size_t index = 0;

                    for (; index < argument_count; index++)
                    {
                        const Tensor& argument = arguments[index];
                        if (IsIdentifier(argument))
                        {
                            new_arguments = arguments;
                            some_substitution = true;
                            break;
                        }
                    }

                    for (; index < argument_count; index++)
                    {
                        const Tensor& argument = arguments[index];
                        if (IsIdentifier(argument))
                        {
                            new_arguments[index] = MakeExpression(builtin_names::AssociativeIdentifier,
                                { argument },
                                argument.GetExpression()->GetMetadata());
                        }
                    }
                }

                if (some_substitution)
                    return MakeExpression(i_candidate.GetExpression()->GetName(), new_arguments, i_candidate.GetExpression()->GetMetadata());
                else
                    return i_candidate;
            });
        }

        void DiscriminationNet::AddPattern(uint32_t i_pattern_id, const Tensor& i_pattern, const Tensor& i_condition)
        {
            const Tensor preprocessed_pattern = PreprocessPattern(i_pattern);

            Edge* last_edge = AddPatternFrom(s_start_node_index, preprocessed_pattern);
            
            // remove the last node added and mark the last edge as leaf
            --m_last_node_index;
            last_edge->m_is_leaf = true;
            last_edge->m_pattern_id = i_pattern_id;
        }

        DiscriminationNet::Edge * DiscriminationNet::AddPatternFrom(uint32_t i_source_node,
            const Tensor & i_pattern)
        {
            const PatternInfo pattern_info = BuildPatternInfo(i_pattern);

            Edge* edge = AddEdge(i_source_node, i_pattern);
            edge->m_function_flags = pattern_info.m_flags;
            edge->m_argument_cardinality |= pattern_info.m_argument_range;
            edge->m_pattern = i_pattern;

            if (!IsLiteral(i_pattern) && !IsIdentifier(i_pattern))
            {
                const Span arguments = i_pattern.GetExpression()->GetArguments();
                for (size_t i = 0; i < arguments.size(); i++)
                {
                    edge = AddPatternFrom(edge->m_dest_node, arguments[i]);
                }
            }
            return edge;
            /*Edge * edge = AddEdge(i_source_node, i_patterns);
            edge->m_function_flags = i_pattern_info.m_flags;
            edge->m_cardinality |= i_pattern_info.m_argument_range;

            edge->m_argument_infos.resize(i_patterns.size());
            for (size_t i = 0; i < i_patterns.size(); i++)
            {
                edge->m_argument_infos[i].m_cardinality |= i_pattern_info.m_arguments[i].m_cardinality;
                edge->m_argument_infos[i].m_remaining |= i_pattern_info.m_arguments[i].m_remaining;
            }

            for (size_t i = 0; i < i_patterns.size(); i++)
            {
                if (!IsLiteral(i_patterns[i]) && !IsIdentifier(i_patterns[i]))
                {
                    //if (!IsRepetition(i_patterns[i]))
                    {
                        const PatternInfo pattern_info = BuildPatternInfo(i_patterns[i]);

                        edge = AddPatternFrom(edge->m_dest_node, i_patterns[i].GetExpression()->GetArguments(), pattern_info);
                    }
                }
            }*/

            return edge;
        }

        DiscriminationNet::Edge * DiscriminationNet::AddEdge(uint32_t i_source_node, const Tensor& i_pattern)
        {
            auto range = m_edges.equal_range(i_source_node);
            for (auto it = range.first; it != range.second; it++)
            {
                if (SamePattern(it->second.m_pattern, i_pattern))
                {
                    return &it->second;
                }
            }

            Edge new_edge;
            new_edge.m_pattern = i_pattern;
            uint32_t new_node = ++m_last_node_index;
            new_edge.m_dest_node = new_node;

            auto res = m_edges.insert(std::pair(i_source_node, std::move(new_edge)));
            return &res->second;
        }

        bool DiscriminationNet::SamePattern(const Tensor& i_first_pattern, const Tensor& i_second_pattern)
    {
            assert(!IsEmpty(i_first_pattern));
            assert(!IsEmpty(i_second_pattern));

            if (IsLiteral(i_first_pattern) || IsIdentifier(i_first_pattern))
            {
                if (!AlwaysEqual(i_first_pattern, i_second_pattern))
                    return false;
            }
            else
            {
                if (i_first_pattern.GetExpression()->GetName() != i_second_pattern.GetExpression()->GetName())
                    return false;
            }
            
            return true;
        }

        std::string DiscriminationNet::ToDotLanguage(std::string_view i_graph_name) const
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

            // nodes
            for(uint32_t i = 0; i <= m_last_node_index; i++)
            {
                dest << "v" << i << "[label = \"";
                if(i == s_start_node_index)
                    dest << "Initial" << escaped_newline;
                else
                    dest << i << escaped_newline;
                dest << "\"]";
                dest.NewLine();
            }

            for (const auto& edge : m_edges)
            {
                if (edge.second.m_is_leaf)
                {
                    dest << "l" << edge.second.m_pattern_id << "[shape = box, label = \"";
                    dest << "Pattern " << edge.second.m_pattern_id << "\"]";
                    dest.NewLine();
                }
            }

            // edges
            std::string text;
            for(const auto & edge : m_edges)
            {
                text = ToSimplifiedStringForm(edge.second.m_pattern);

                /*text = "{" + edge.second.m_cardinality.ToString() + "}" + escaped_newline;

                for (size_t i = 0; i < edge.second.m_patterns.size(); i++)
                {
                    const Tensor& pattern = edge.second.m_patterns[i];

                    if (IsLiteral(pattern) || IsIdentifier(pattern) || IsRepetition(pattern))
                        text += ToSimplifiedStringForm(pattern);
                    else
                        text += pattern.GetExpression()->GetName().AsString();
                    
                    if (edge.second.m_argument_infos[i].m_cardinality != Range{ 1, 1 })
                        text += " - card: " + edge.second.m_argument_infos[i].m_cardinality.ToString();

                    if (!edge.second.m_argument_infos[i].m_remaining.IsEmpty())
                        text += ", rem: " + edge.second.m_argument_infos[i].m_remaining.ToString();

                    text += "\\l";
                }*/

                std::string color = "black";
                std::string dest_node = edge.second.m_is_leaf ? ToString("l", edge.second.m_pattern_id) : ToString("v", edge.second.m_dest_node);

                dest << 'v' << edge.first << " -> " << dest_node
                    << "[style=\"solid\", color=\"" << color << "\", label=\"" << text << "\"]" << ';';

                dest.NewLine();
            }

            dest.Untab();
            dest << "}";
            dest.NewLine();

            return dest.StealString();
        }

    } // namespace pattern

} // namespace djup
