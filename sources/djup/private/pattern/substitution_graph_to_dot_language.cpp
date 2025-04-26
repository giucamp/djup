
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <private/pattern/substitution_graph.h>
#include <private/pattern/discrimination_tree.h>
#include <private/pattern/candidate.h>
#include <private/pattern/debug_utils.h>

namespace djup
{
    namespace pattern
    {
        namespace
        {
            static std::string TensorListToString(Span<const Tensor> i_tensors)
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
            
            for (uint32_t node_index = 0; node_index < m_solution_node_count; node_index++)
            {
                auto & node = graph.AddNode(ToString(node_index));

                bool is_node_to_expand = AnyOf(m_discr_nodes_to_expand, 
                    [node_index](auto& node_to_exand) 
                    { return node_to_exand.m_node == node_index; });
                if (is_node_to_expand)
                    node.SetFillColor({ 255, 255, 100 });
                else
                    node.SetFillColor({ 255, 255, 255 });

                if (node_index == m_discrimination_tree.GetRootNodeIndex())
                {
                    node.SetShape(GraphWizGraph::NodeShape::Box);
                    node.SetFillColor({ 235, 200, 255 });;
                }
                else if (node_index < static_cast<uint32_t>(m_discrimination_tree.GetNodeCount()) &&
                    m_discrimination_tree.IsLeafNode(node_index))
                {
                    // leaf node, draw as square
                    node.SetShape(GraphWizGraph::NodeShape::Box);

                    // leaf nodes have a different label
                    std::string text = ToString("Pattern ", 
                        m_discrimination_tree.GetPatternId(node_index), " - node ",  node_index);
                    #if DJUP_DEBUG_DISCRIMINATION_TREE
                        text += "\n" + m_discrimination_tree.DbgGetFullPattern(node_index);
                    #endif
                    node.SetLabel(text);
                }
            }

            // discrimination edges
            for (const auto& pair : m_discrimination_tree.GetEdgeMap())
            {
                const uint32_t source_node = pair.first;
                const DiscriminationTree::Edge & edge = pair.second;

                std::string text = TensorSpanToString(edge.m_labels, FormatFlags::Tidy, 1);
               
                std::string non_tidy_text = TensorSpanToString(edge.m_labels, {}, 1);
                if (non_tidy_text != text)
                    text += "\n" + non_tidy_text;
                
                if (!edge.m_pattern_info.m_labels_range.HasSingleValue())
                {
                    text += "\n{" + edge.m_pattern_info.m_labels_range.ToString() + "}";
                }

                graph.AddEdge(source_node, edge.m_dest_node, text);
            }

            // solution edges
            for (const auto & pair : m_solution_graph)
            {
                const uint32_t source_node = pair.first;
                const SolutionEdge & edge = pair.second;

                graph.AddEdge(source_node, edge.m_dest)
                    .SetDrawingColor({0, 0, 255});
            }

            // candidates
            for (const auto& candidate : m_candidate_edges)
            {
                std::string text = TensorListToString(candidate.m_targets);
                graph.AddEdge(candidate.m_source_node, candidate.m_dest_node, text)
                    .SetStyle(GraphWizGraph::EdgeStyle::Dotted);
            }

            /*using Handle = Pool<CandidateEdge>::Handle;
            
            struct HandleHash
            {
                std::size_t operator()(const Handle& i_source) const noexcept
                {
                    return i_source.m_index ^ i_source.m_version; // or use boost::hash_combine
                }
            };
            
            std::unordered_map<Handle, uint32_t, HandleHash> handles;

            uint32_t index = 0;
            for (const Candidate& candidate : m_candidates)
            {
                auto label = ToString(
                    candidate.m_source_discr_node, " -> ", candidate.m_edge->m_dest_node
                );
                graph.AddNode(label);
                handles.insert(std::make_pair(m_candidates.HandleOf(candidate), index));

                ++index;
            }

            index = 0;
            for (const auto & handle_it : handles)
            {
                const Candidate& candidate = m_candidates.GetObject(handle_it.first);

                auto from_it = handles.find(candidate.m_parent_candidate);
                auto to_it = handles.find(handle_it.first);

                if (from_it != handles.end() && to_it != handles.end())
                {
                    graph.AddEdge(from_it->second, to_it->second);
                }
                ++index;
            }*/

            return graph;
        }

    } // namespace pattern

} // namespace djup
