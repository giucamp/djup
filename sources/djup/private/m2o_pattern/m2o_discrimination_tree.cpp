
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <private/m2o_pattern/m2o_discrimination_tree.h>
#include <private/m2o_pattern/m2o_pattern_info.h>
#include <private/m2o_pattern/m2o_debug_utils.h>
#include <private/builtin_names.h>
#include <djup/expression.h>
#include <private/substitute_by_predicate.h>
#include <core/flags.h>
#include <core/to_string.h>

namespace djup
{
    namespace m2o_pattern
    {
        namespace
        {
            Tensor PreprocessPattern(const Tensor& i_pattern)
            {
                return SubstituteByPredicate(i_pattern, [](const Tensor& i_candidate) {
                    FunctionFlags flags = GetFunctionFlags(*i_candidate.GetExpression());

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
                            if (!IsConstant(argument))
                            {
                                new_arguments = arguments;
                                some_substitution = true;
                                break;
                            }
                        }

                        for (; index < argument_count; index++)
                        {
                            const Tensor& argument = arguments[index];
                            if (!IsConstant(argument))
                            {
                                new_arguments[index] = MakeExpression(
                                    argument.GetExpression()->GetType(),
                                    builtin_names::AssociativeIdentifier,
                                    { argument },
                                    argument.GetExpression()->GetMetadata());
                            }
                        }
                    }

                    if (some_substitution)
                        return MakeExpression(
                            i_candidate.GetExpression()->GetType(),
                            i_candidate.GetExpression()->GetName(), 
                            new_arguments, i_candidate.GetExpression()->GetMetadata());
                    else
                        return i_candidate;
                    });
                }
        }

        DiscriminationTree::DiscriminationTree()
        {
            
        }

        uint32_t DiscriminationTree::NewNode()
        {
            const uint32_t new_node = m_node_count++;
            return new_node;
        }

        /** This is the entry point to add a pattern */
        void DiscriminationTree::AddPattern(uint32_t i_pattern_id,
            const Tensor& i_pattern, const Tensor& i_condition)
        {
            DJUP_ASSERT(i_pattern_id >= 0);

            const Tensor preprocessed = PreprocessPattern(i_pattern);

            // AddEdje(GetRootNodeIndex(), preprocessed);
        }

        /*const DiscriminationTree::Edge & DiscriminationTree::AddEdje(uint32_t i_source_node, const Tensor & i_pattern)
        {
            const PatternInfo pattern_info = BuildPatternInfo(i_pattern);
            const ExpressionKind expression_kind = GetExpressionKind(*i_pattern.GetExpression());

            auto range = m_edges.equal_range(i_source_node);
            for (auto it = range.first; it != range.second; ++it)
            {
                const Edge & existing_edge = it->second;

                switch (expression_kind)
                {
                case ExpressionKind::Constant:
                    if (AlwaysEqual(existing_edge.m_pattern, i_pattern))
                    {
                        return existing_edge;
                    }
                    break;

                case ExpressionKind::Identifier:
                    if (existing_edge.m_pattern.GetExpression()->GetType() == 
                        i_pattern.GetExpression()->GetType())
                    {
                        return existing_edge;
                    }
                    break;

                case ExpressionKind::VariableFunction:
                    if (existing_edge.m_pattern.GetExpression()->GetName() ==
                        i_pattern.GetExpression()->GetName())
                    {

                    }
                    break;

                case ExpressionKind::Variadic:
                    break;

                default:
                    DJUP_ASSERT(false);
                    break;
                }
               
            }

            const uint32_t new_node = NewNode();
            return new_node;

        }*/

        GraphWizGraph DiscriminationTree::ToGraphWiz(std::string_view i_graph_name) const
        {
            GraphWizGraph graph(i_graph_name);



            return graph;
        }

    } // namespace m2o_pattern

} // namespace djup
