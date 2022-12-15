
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

        void DiscriminationNet::AddPattern(uint32_t i_pattern_id, const Tensor & i_pattern, const Tensor & i_condition)
        {
            ArgumentInfo argument_info;
            argument_info.m_cardinality = {1, 1};
            argument_info.m_remaining = {0, 0};
            uint32_t last_node = AddPatternFrom(s_start_node_index, PreprocessPattern(i_pattern), argument_info, i_condition);

            Edge * leaf_edge = AddEdge(last_node, {});
            leaf_edge->m_pattern_id = i_pattern_id;
        }

        uint32_t DiscriminationNet::AddPatternFrom(uint32_t i_source_node, 
            const Tensor & i_pattern, const ArgumentInfo & i_argument_info, const Tensor & i_condition)
        {
            assert(!IsEmpty(i_pattern));

            const PatternInfo pattern_info = BuildPatternInfo(i_pattern);

            Edge * edge = AddEdge(i_source_node, i_pattern);
            edge->m_function_flags = GetFunctionFlags(i_pattern.GetExpression()->GetName());
            edge->m_info.m_cardinality |= i_argument_info.m_cardinality;
            edge->m_info.m_remaining |= i_argument_info.m_remaining;
            uint32_t curr_node = edge->m_dest_node;

            if(!IsConstant(i_pattern) && !IsIdentifier(i_pattern))
            {
                edge->m_argument_cardinality |= pattern_info.m_argument_range;

                Span<const Tensor> parameters = i_pattern.GetExpression()->GetArguments();
                for(size_t i = 0; i < parameters.size(); i++)
                {
                    curr_node = AddPatternFrom(curr_node, parameters[i], pattern_info.m_arguments[i], i_condition);
                }
            }

            return curr_node;
        }

        DiscriminationNet::Edge * DiscriminationNet::AddEdge(uint32_t i_source_node, const Tensor & i_expression)
        {
            auto range = m_edges.equal_range(i_source_node);
            for(auto it = range.first; it != range.second; it++)
            {
                bool same;
                if(IsConstant(it->second.m_expression) || IsIdentifier(it->second.m_expression))
                    same = AlwaysEqual(it->second.m_expression, i_expression);
                else
                    same = it->second.m_expression.GetExpression()->GetName() == i_expression.GetExpression()->GetName();

                if(same)
                {
                    return &it->second;
                }
            }

            Edge new_edge;

            if (!IsEmpty(i_expression)) // if it's not a leaf edge
            {
                new_edge.m_expression = i_expression;
                uint32_t new_node = ++m_last_node_index;
                new_edge.m_dest_node = new_node;
            }
            
            auto res = m_edges.insert(std::pair(i_source_node, std::move(new_edge)));
            return &res->second;
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
                    dest << "Node " << i << escaped_newline;
                dest << "\"]";
                dest.NewLine();
            }

            // leaf nodes
            for (const auto& edge : m_edges)
            {
                if (IsEmpty(edge.second.m_expression))
                {
                    dest << "l" << edge.second.m_pattern_id << "[shape = box, label = \"";
                    dest << edge.second.m_pattern_id << "\"]";

                    dest.NewLine();
                }
            }

            // edges
            for(const auto & edge : m_edges)
            {
                if (!IsEmpty(edge.second.m_expression))
                {
                    std::string name;
                    if (IsConstant(edge.second.m_expression) || IsIdentifier(edge.second.m_expression))
                        name = ToSimplifiedStringForm(edge.second.m_expression);
                    else
                        name = edge.second.m_expression.GetExpression()->GetName().AsString();

                    if (edge.second.m_info.m_cardinality != Range{ 1, 1 })  
                        name += escaped_newline + "card: " + edge.second.m_info.m_cardinality.ToString();

                    if (!edge.second.m_argument_cardinality.IsEmpty())
                        name += escaped_newline + "args: " + edge.second.m_argument_cardinality.ToString();

                    name += escaped_newline + "rem: " + edge.second.m_info.m_remaining.ToString();

                    std::string color = "black";

                    dest << 'v' << edge.first << " -> v" << edge.second.m_dest_node
                        << "[style=\"solid\", color=\"" << color << "\", label=\"" << name << "\"]" << ';';
                }
                else
                {
                    std::string color = "black";

                    dest << 'v' << edge.first << " -> l" << edge.second.m_pattern_id
                        << "[style=\"dashed\", color=\"" << color << "\"]" << ';';
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
