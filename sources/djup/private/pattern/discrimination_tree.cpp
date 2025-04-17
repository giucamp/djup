
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/pattern/discrimination_tree.h>
#include <private/pattern/pattern_info.h>
#include <private/pattern/debug_utils.h>
#include <private/builtin_names.h>
#include <private/expression.h>
#include <private/substitute_by_predicate.h>
#include <core/flags.h>
#include <core/to_string.h>

namespace djup
{
    namespace pattern
    {
        namespace
        {
            template< typename... ARGS>
            void DiscrTreeDebugPrintLn(ARGS&&... args)
            {
#if !defined(DJUP_DEBUG_DISCRIMINATION_TREE)
#error DJUP_DEBUG_DISCRIMINATION_TREE must be defined
#endif
#if DJUP_DEBUG_DISCRIMINATION_TREE
                PrintLn(args...);
#endif
            }
        }

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

        DiscriminationTree::DiscriminationTree()
        {
            NewNode();
        }

        /** This is the entry point to add a pattern */
        void DiscriminationTree::AddPattern(uint32_t i_pattern_id,
            const Tensor& i_pattern, const Tensor& i_condition)
        {
            assert(i_pattern_id >= 0);

            const Tensor preprocessed = PreprocessPattern(i_pattern);

            // pattern info for the first level node (the one that starts from the root)
            PatternInfo pattern_info;
            pattern_info.m_labels_range = { 1, 1 };
            pattern_info.m_labels_info.resize(1);
            pattern_info.m_labels_info[0].m_cardinality = { 1, 1 };
            pattern_info.m_labels_info[0].m_remaining = { 0, 0 };
            pattern_info.m_flags = {};
            #if !defined(DJUP_DEBUG_DISCRIMINATION_TREE)
                #error DJUP_DEBUG_DISCRIMINATION_TREE must be defined
            #endif
            #if DJUP_DEBUG_PATTERN_INFO
                pattern_info.m_dbg_pattern = i_pattern;
            #endif

            Edge* curr_edge = AddEdge(s_root_node_index, { &preprocessed, 1 }, pattern_info);

            ProcessPattern(curr_edge->m_dest_node,
                preprocessed.GetExpression()->GetArguments(), BuildPatternInfo(i_pattern));

            assert(m_leaf_pattern_id.back() == -1);
            m_leaf_pattern_id.back() = i_pattern_id;

            #if !defined(DJUP_DEBUG_DISCRIMINATION_TREE)
                #error DJUP_DEBUG_DISCRIMINATION_TREE must be defined
            #endif
            #if DJUP_DEBUG_DISCRIMINATION_TREE
                m_dbg_full_patterns[i_pattern_id] = ToSimplifiedStringForm(i_pattern);
            #endif
        }

        DiscriminationTree::Edge* DiscriminationTree::ProcessPattern(
            int32_t i_source_node, Span<const Tensor> i_patterns, const PatternInfo& i_pattern_info)
        {
            Edge* curr_edge = AddEdge(i_source_node, i_patterns, i_pattern_info);

            // process arguments
            for (size_t i = 0; i < i_patterns.size(); i++)
            {
                // literals and identifiers are considered terminal nodes
                if (!IsLiteral(i_patterns[i]) && !IsIdentifier(i_patterns[i]))
                {
                    Span<const Tensor> arguments = i_patterns[i].GetExpression()->GetArguments();
                    if (!arguments.empty())
                    {
                        curr_edge = ProcessPattern(curr_edge->m_dest_node,
                            arguments, BuildPatternInfo(i_patterns[i]));
                    }
                }
            }

            return curr_edge;
        }

        /** Adds an edge to a new node, or returns an identical one */
        DiscriminationTree::Edge* DiscriminationTree::AddEdge(
            uint32_t i_source_node, Span<const Tensor> i_patterns,
            const PatternInfo& i_source_pattern_info)
        {
            DiscrTreeDebugPrintLn("Adding edge from ", i_source_node,
                " with label ", TensorSpanToString(i_patterns, 1));

            // search for an identical edge with the same patterns
            auto range = m_edges.equal_range(i_source_node);
            for (auto it = range.first; it != range.second; it++)
            {
                Edge& existing_edge = it->second;
                const bool same = SameRootPatterns(existing_edge.m_labels, i_patterns);
                DiscrTreeDebugPrintLn("Comparing ", TensorSpanToString(existing_edge.m_labels),
                    " to ", TensorSpanToString(i_patterns), ": ", same);
                if (same)
                {
                    // merge the cardinalities in the exiting edge
                    existing_edge.m_pattern_info.m_flags = i_source_pattern_info.m_flags;
                    existing_edge.m_pattern_info.m_labels_range |= i_source_pattern_info.m_labels_range;
                    existing_edge.m_pattern_info.m_labels_info.resize(existing_edge.m_pattern_info.m_labels_info.size());
                    for (size_t i = 0; i < existing_edge.m_labels.size(); i++)
                    {
                        existing_edge.m_pattern_info.m_labels_info[i].m_cardinality |=
                            i_source_pattern_info.m_labels_info[i].m_cardinality;
                        existing_edge.m_pattern_info.m_labels_info[i].m_remaining |=
                            i_source_pattern_info.m_labels_info[i].m_remaining;
                    }

                    return &existing_edge;
                }
            }

            // new node
            uint32_t new_node = NewNode();

            // new edge
            Edge new_edge;
            new_edge.m_pattern_info = i_source_pattern_info;
            new_edge.m_labels.assign(i_patterns.begin(), i_patterns.end());
            new_edge.m_dest_node = new_node;
            auto res = m_edges.insert(std::pair(i_source_node, std::move(new_edge)));
            return &res->second;
        }

        int32_t DiscriminationTree::NewNode()
        {
            uint32_t new_node = m_node_count++;
            m_leaf_pattern_id.push_back(-1);
            assert(m_leaf_pattern_id.size() == m_node_count);
            DiscrTreeDebugPrintLn("\tnew node: ", new_node);
            return new_node;
        }

        /** Checks whether two pattern list are equal */
        bool DiscriminationTree::SameRootPatterns(Span<const Tensor> i_first_patterns, Span<const Tensor> i_second_patterns)
        {
            if (i_first_patterns.size() != i_second_patterns.size())
                return false;

            for (size_t i = 0; i < i_first_patterns.size(); i++)
            {
                assert(!IsEmpty(i_first_patterns[i]));
                assert(!IsEmpty(i_second_patterns[i]));

                if (IsLiteral(i_first_patterns[i]) || IsIdentifier(i_first_patterns[i]))
                {
                    if (!AlwaysEqual(i_first_patterns[i], i_second_patterns[i]))
                        return false;
                }
                else
                {
                    if (i_first_patterns[i].GetExpression()->GetName() != i_second_patterns[i].GetExpression()->GetName())
                        return false;
                }
            }
            
            return true;
        }

        GraphWizGraph DiscriminationTree::ToGraphWiz(std::string_view i_graph_name) const
        {
            DiscrTreeDebugPrintLn("---------------------------");
            for (const auto& edge : m_edges)
            {
                std::string dest = std::to_string(edge.second.m_dest_node);
                if (IsLeafNode(edge.first))
                    dest += "(Leaf)";
                DiscrTreeDebugPrintLn(edge.first, "->", dest, ": ", 
                    TensorSpanToString(Span(edge.second.m_labels), 1));
            }
            DiscrTreeDebugPrintLn("---------------------------");

            GraphWizGraph graph(i_graph_name);

            for (int32_t i = 0; i < m_node_count; i++)
            {
                if (i == s_root_node_index)
                {
                    graph.SetNodeShape(core::GraphWizGraph::Box);
                    graph.SetFillColor(235, 200, 255);
                    graph.AddNode("Root");
                }
                else if (IsLeafNode(i))
                {
                    std::string text = ToString("Pattern ", m_leaf_pattern_id[i], " - node ",  i);
                    #if !defined(DJUP_DEBUG_DISCRIMINATION_TREE)
                        #error DJUP_DEBUG_DISCRIMINATION_TREE must be defined
                    #endif
                    #if DJUP_DEBUG_DISCRIMINATION_TREE
                        const auto full_pattern_it = m_dbg_full_patterns.find(m_leaf_pattern_id[i]);
                        assert(full_pattern_it != m_dbg_full_patterns.end());
                        text += "\n" + full_pattern_it->second;
                    #endif

                    graph.SetNodeShape(core::GraphWizGraph::Box);
                    graph.SetFillColor(235, 200, 255);
                    graph.AddNode(text);
                }
                else
                {
                    graph.SetNodeShape(core::GraphWizGraph::Ellipse);
                    graph.SetFillColor(200, 230, 255);
                    graph.AddNode(ToString(i));
                }
            }

            // edges
            for (const auto & edge : m_edges)
            {
                std::string text = TensorSpanToString(edge.second.m_labels, 1, true);
                if (!edge.second.m_pattern_info.m_labels_range.HasSingleValue())
                    text += "\n{" + edge.second.m_pattern_info.m_labels_range.ToString() + "}";

                graph.AddEdge(edge.first, edge.second.m_dest_node, text);
            }

            return graph;
        }

    } // namespace pattern

} // namespace djup
