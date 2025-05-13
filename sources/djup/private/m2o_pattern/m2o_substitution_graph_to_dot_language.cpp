
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <private/m2o_pattern/m2o_substitution_graph.h>
#include <private/m2o_pattern/m2o_discrimination_tree.h>
#include <private/m2o_pattern/m2o_candidate.h>
#include <private/m2o_pattern/m2o_debug_utils.h>

namespace djup
{
    namespace m2o_pattern
    {
        namespace
        {
            std::string TensorListToString(Span<const Tensor> i_tensors)
            {
                std::string result;
                for (size_t i = 0; i < i_tensors.size(); i++)
                {
                    if (i)
                        result += ", ";
                    result += ToSimplifiedString(i_tensors[i]);
                }
                return result;
            }
        }

        GraphWizGraph SubstitutionGraph::ToDotGraphWiz(std::string_view i_graph_name) const
        {
            GraphWizGraph graph(i_graph_name);
            
            /*for (uint32_t node_index = 0; node_index < m_solution_node_count; node_index++)
            {
                const bool is_node_to_expand = AnyOf(m_discr_nodes_to_expand,
                    [node_index](auto& node_to_exand)
                    { return node_to_exand.m_node == node_index; });

                const bool is_root = node_index == m_discrimination_tree.GetRootNodeIndex();

                const bool is_leaf_node = node_index < m_discrimination_tree.GetNodeCount() &&
                    m_discrimination_tree.IsLeafNode(node_index);

                GraphWizGraph::Node& node = graph.AddNode({});

                if (is_root || is_leaf_node)
                    node.SetShape(GraphWizGraph::NodeShape::Box);

                if(is_node_to_expand)
                    node.SetFillColor({ 255, 255, 100 });

                std::string label;
                if (is_leaf_node)
                {
                    std::string text = ToString("Pattern ", 
                        m_discrimination_tree.GetPatternId(node_index), " - node ",  node_index);
                    #if DJUP_DEBUG_DISCRIMINATION_TREE
                        text += "\n" + m_discrimination_tree.DbgGetFullPattern(node_index);
                    #endif
                    node.SetLabel(std::move(text));
                }
                else if (is_root)
                {
                    node.SetLabel("Root");
                }
                else
                {
                    node.SetLabel(ToString(node_index));
                }
            }

            // discrimination edges
            for (const auto& pair : m_discrimination_tree.GetEdgeMap())
            {
                const uint32_t source_node = pair.first;
                const DiscriminationTree::Edge & edge = pair.second;

                std::string text = TensorSpanToString(edge.m_patterns, FormatFlags::Tidy, 1);
               
                std::string non_tidy_text = TensorSpanToString(edge.m_patterns, {}, 1);
                if (non_tidy_text != text)
                    text += "\n" + non_tidy_text;
                
                if (!edge.m_pattern_info.m_arguments_range.HasSingleValue())
                {
                    text += "\n{" + edge.m_pattern_info.m_arguments_range.ToString() + "}";
                }

                graph.AddEdge(source_node, edge.m_dest_node, text);
            }

            // solution edges
            for (const auto & pair : m_solution_graph)
            {
                const uint32_t source_node = pair.first;
                const SolutionEdge & edge = pair.second;

                std::string text;
                for (size_t i = 0; i < edge.m_substitutions.size(); ++i)
                {
                    if (i != 0)
                        text += "\n";
                    text += 
                        edge.m_substitutions[i].m_identifier_name.AsString() + 
                        " = " +
                        ToSimplifiedString(edge.m_substitutions[i].m_value);
                }

                if (edge.m_open != 0 || edge.m_close != 0)
                    text += "\n";
                if (edge.m_open != 0)
                    text += std::string(edge.m_open, '+');
                if (edge.m_close != 0)
                    text += std::string(edge.m_close, '-');

                graph.AddEdge(source_node, edge.m_dest, text)
                    .SetDrawingColor({0, 0, 210 })
                    .SetFontColor({ 0, 0, 210 });
            }

            // candidates
            for (const auto& candidate : m_candidate_edges)
            {
                std::string text = TensorListToString(candidate.m_targets);
                text += "\nis\n";
                text += TensorListToString(candidate.m_patterns);
                if (candidate.m_repetitions != 1)
                    text += " (" + ToString(candidate.m_repetitions) + " times)";

                if (candidate.m_open != 0 || candidate.m_close != 0)
                    text += "\n";
                if (candidate.m_open != 0)
                    text += std::string(candidate.m_open, '+');
                if (candidate.m_close != 0)
                    text += std::string(candidate.m_close, '-');

                graph.AddEdge(candidate.m_source_node, candidate.m_dest_node, text)
                    .SetStyle(GraphWizGraph::EdgeStyle::Dotted);
            }*/

            return graph;
        }

    } // namespace m2o_pattern

} // namespace djup
