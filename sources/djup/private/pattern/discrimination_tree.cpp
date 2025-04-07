
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
#include <algorithm> // for min, max

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

        /** This is the entry point to add a pattern */
        void DiscriminationTree::AddPattern(uint32_t i_pattern_id,
            const Tensor& i_pattern, const Tensor& i_condition)
        {
            const Tensor preprocessed = PreprocessPattern(i_pattern);

            // pattern info for the level node (the one that starts from the root)
            PatternInfo pattern_info;
            pattern_info.m_labels_range = {1, 1};
            pattern_info.m_labels_info.resize(1);
            pattern_info.m_labels_info[0].m_cardinality = { 1, 1 };
            pattern_info.m_labels_info[0].m_remaining = { 0, 0 };
            pattern_info.m_dbg_pattern = i_pattern;
            pattern_info.m_flags = {};

            Edge* curr_edge = AddEdge(s_root_node_index, { &preprocessed, 1 }, pattern_info);

            ProcessPattern(curr_edge->m_dest_node, preprocessed.GetExpression()->GetArguments(), BuildPatternInfo(i_pattern));

            curr_edge->is_leaf_node = true;

            DiscrTreeDebugPrintLn("Leaf node");
        }

        DiscriminationTree::Edge* DiscriminationTree::ProcessPattern(
            int32_t i_source_node, Span<const Tensor> i_patterns, const PatternInfo & i_pattern_info)
        {
            Edge * curr_edge = AddEdge(i_source_node, i_patterns, i_pattern_info);

            // ----------------------------------------------
            for (size_t i = 0; i < i_patterns.size(); i++)
            {
                if (!IsLiteral(i_patterns[i]) && !IsIdentifier(i_patterns[i]))
                {
                    Span<const Tensor> arguments = i_patterns[i].GetExpression()->GetArguments();
                    
                    const PatternInfo pattern_info = BuildPatternInfo(i_patterns[i]);

                    curr_edge = AddEdge(curr_edge->m_dest_node, 
                        arguments, pattern_info);
                }
            }
            // -------------------------------------------------

            return curr_edge;
        }

        /** Adds an edge, or returns an identical one */
        DiscriminationTree::Edge* DiscriminationTree::AddEdge(
            uint32_t i_source_node, Span<const Tensor> i_patterns,
            const PatternInfo& i_source_pattern_info)
        {
            DiscrTreeDebugPrintLn("Adding edge from ", i_source_node, " with ", TensorSpanToString(i_patterns, 1));

            // search for an identical edge with the same patterns
            auto range = m_edges.equal_range(i_source_node);
            for (auto it = range.first; it != range.second; it++)
            {
                Edge& existing_edge = it->second;
                if (SamePatterns(existing_edge.m_labels, i_patterns))
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

            // insert and setup a new edge
            uint32_t new_node = ++m_last_node_index;
            Edge new_edge;
            new_edge.m_pattern_info = i_source_pattern_info;
            new_edge.m_labels.assign(i_patterns.begin(), i_patterns.end());
            new_edge.m_dest_node = new_node;
            auto res = m_edges.insert(std::pair(i_source_node, std::move(new_edge)));
            return &res->second;
        }

        /** Checks whether two pattern list are equal */
        bool DiscriminationTree::SamePatterns(Span<const Tensor> i_first_patterns, Span<const Tensor> i_second_patterns)
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
        
        const DiscriminationTree::Edge* DiscriminationTree::GetRootNode() const
        {
            auto const root_edje_it = std::as_const(m_edges).find(0);
            if (root_edje_it == m_edges.end())
            {
                // empty tree
                return nullptr;
            }
            return &root_edje_it->second;
        }

        const DiscriminationTree::Edge& DiscriminationTree::GetNodeFrom(uint32_t i_source_node) const
        {
            auto const edge_it = std::as_const(m_edges).find(i_source_node);
            if (edge_it == m_edges.end())
            {
                Error("Discrimination tree: non-existing node");
            }
            return edge_it->second;
        }

        std::string DiscriminationTree::ToDotLanguage(std::string_view i_graph_name) const
        {
            DiscrTreeDebugPrintLn("---------------------------");
            for (const auto& edge : m_edges)
            {
                DiscrTreeDebugPrintLn(edge.first, ": ", TensorSpanToString(Span(edge.second.m_labels), 1), 
                    ", ", edge.second.is_leaf_node);
            }
            DiscrTreeDebugPrintLn("---------------------------");

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
                dest << "v" << i;
                if (i == s_root_node_index)
                    dest << "[shape = box]";

                dest << "[label = \"";
                dest << i;
                dest << escaped_newline << "\"]";
                dest.NewLine();
            }

            // edges
            for(const auto & edge : m_edges)
            {
                std::string text = TensorSpanToString(edge.second.m_labels, 1, true);
                
                // uncomment the following to see the raw form of edge patterns
                // text += escaped_newline + TensorSpanToString(edge.second.m_arguments, 1, false);

                std::string color = "black";
                std::string dest_node = 
                    ToString("v", edge.second.m_dest_node);
                if (edge.second.m_pattern_info.m_labels_range != Range{ 1, 1 })
                    text += escaped_newline + "{" + edge.second.m_pattern_info.m_labels_range.ToString() + "}";

                dest << 'v' << edge.first << " -> " << dest_node;
                dest << "[style=\"solid\", color=\"" << color << "\", label=\"" << text;
                dest << "\"]" << ';';

                if (edge.second.is_leaf_node)
                {
                    // draw pattern box
                    dest << "v" << edge.second.m_dest_node;
                    dest << "[shape = box, label = \"";
                    dest << "Pattern " << edge.second.m_dest_node << "\"]";
                    dest.NewLine();
                }

                dest.NewLine();
            }

            dest.Untab();
            dest << "}";
            dest.NewLine();

            std::string dot = dest.StealString();
            return dot;
        }

    } // namespace pattern

} // namespace djup
