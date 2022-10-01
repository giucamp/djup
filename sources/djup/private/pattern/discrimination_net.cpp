
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/pattern/discrimination_net.h>
#include <private/pattern/pattern_info.h>
#include <private/builtin_names.h>
#include <private/expression.h>
#include <core/flags.h>
#include <core/to_string.h>
#include <algorithm> // for min, max

namespace djup
{
    namespace pattern
    {
        void DiscriminationNet::AddPattern(uint32_t i_pattern_id, const Tensor & i_pattern, const Tensor & i_condition)
        {
            ArgumentInfo argument_info;
            argument_info.m_cardinality = {1, 1};
            argument_info.m_remaining = {0, 0};
            AddPatternFrom(i_pattern_id, s_start_node_index, i_pattern, argument_info, i_condition);
        }

        uint32_t DiscriminationNet::AddPatternFrom(uint32_t i_pattern_id, 
            uint32_t i_source_node, const Tensor & i_pattern, const ArgumentInfo & i_argument_info, const Tensor & i_condition)
        {
            const PatternInfo pattern_info = BuildPatternInfo(i_pattern);

            Edge * edge = AddEdge(i_source_node, i_pattern);
            edge->m_function_flags = GetFunctionFlags(i_pattern.GetExpression()->GetName());
            edge->m_cardinality = i_argument_info.m_cardinality;
            edge->m_remaining_targets = i_argument_info.m_remaining;
            edge->m_argument_cardinality = {0, 0};
            uint32_t curr_node = edge->m_dest_node;

            if(!IsConstant(i_pattern) && !IsIdentifier(i_pattern))
            {
                edge->m_argument_cardinality = pattern_info.m_argument_range;

                Span<const Tensor> parameters = i_pattern.GetExpression()->GetArguments();
                for(size_t i = 0; i < parameters.size(); i++)
                {
                    curr_node = AddPatternFrom(i_pattern_id, curr_node, parameters[i], pattern_info.m_arguments[i], i_condition);
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

            uint32_t new_node = ++m_last_node_index;

            Edge new_edge;
            new_edge.m_expression = i_expression;
            new_edge.m_dest_node = new_node;
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

            for(const auto & edge : m_edges)
            {
                std::string name;
                if(IsConstant(edge.second.m_expression) || IsIdentifier(edge.second.m_expression))
                    name = ToSimplifiedStringForm(edge.second.m_expression);
                else
                    name = edge.second.m_expression.GetExpression()->GetName().AsString();

                if(edge.second.m_cardinality != Range{1, 1})
                    name += escaped_newline + "card: " + edge.second.m_cardinality.ToString();

                if(edge.second.m_argument_cardinality != Range{0, 0})
                    name += escaped_newline + "args: " + edge.second.m_argument_cardinality.ToString();

                name += escaped_newline + "rem: " + edge.second.m_remaining_targets.ToString();

                std::string color = "black";

                dest << 'v' << edge.first << " -> v" << edge.second.m_dest_node
                    << "[style=\"dashed\", color=\"" << color << "\", label=\"" << name << "\"]"  << ';';

                dest.NewLine();
            }

            dest.Untab();
            dest << "}";
            dest.NewLine();

            return dest.StealString();
        }

    } // namespace pattern

} // namespace djup
