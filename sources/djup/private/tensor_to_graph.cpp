
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <djup/tensor.h>
#include <djup/expression.h>
#include <private/builtin_names.h>
#include <core/graph_wiz.h>
#include <core/flags.h>

namespace djup
{
    namespace
    {
        struct ExpressionToGraphContext
        {
            GraphWizGraph m_graph;
            int32_t m_node_count{};
        };

        int32_t TensorToGraph(ExpressionToGraphContext & i_context,
            const Expression& i_source, FormatFlags i_format_flags, size_t i_depth)
        {
            uint32_t vertex_index;
            
            if (HasFlag(i_format_flags, FormatFlags::Tidy))
            {
                if (i_source.GetName() == builtin_names::Identifier)
                {
                    std::string identifier_name;
                    const Expression& first_arg = *i_source.GetArgument(0).GetExpression();
                    const Expression& second_arg = *i_source.GetArgument(1).GetExpression();
                    DJUP_ASSERT(first_arg.GetName() == builtin_names::TensorType);

                    identifier_name += first_arg.GetArgument(0).GetExpression()->GetName().AsString();
                    if (!second_arg.GetName().IsEmpty())
                    {
                        identifier_name += ' ';
                        identifier_name += second_arg.GetName().AsString();
                    }
                    vertex_index = i_context.m_graph.AddNode(identifier_name);
                }
                else if (i_source.GetName() == builtin_names::Literal)
                {
                    const std::string literal = i_source.GetArgument(0).GetExpression()->GetName().AsString();
                    vertex_index = i_context.m_graph.AddNode(literal);
                }
                else
                {
                    vertex_index = i_context.m_graph.
                        AddNode(i_source.GetName().AsString());

                    if (i_depth > 0)
                    {
                        for (const Tensor& argument : i_source.GetArguments())
                        {
                            uint32_t arg_vertex = TensorToGraph(
                                i_context, *argument.GetExpression(),
                                i_format_flags, i_depth - 1);
                            i_context.m_graph.AddEdge(vertex_index, arg_vertex);
                        }
                    }
                }
            }
            else
            {
                vertex_index = i_context.m_graph.
                    AddNode(i_source.GetName().AsString());

                if (i_depth >= 0)
                {
                    for (const Tensor& argument : i_source.GetArguments())
                    {
                        uint32_t arg_vertex = TensorToGraph(
                            i_context, *argument.GetExpression(),
                            i_format_flags, i_depth - 1);
                        i_context.m_graph.AddEdge(vertex_index, arg_vertex);
                    }
                }
            }

            ++i_context.m_node_count;

            return vertex_index;
        }

        GraphWizGraph TensorToGraph(const Expression& i_source,
            FormatFlags i_format_flags, size_t i_depth)
        {
            ExpressionToGraphContext context;
            TensorToGraph(context, i_source, i_format_flags, i_depth);
            return context.m_graph;
        }

    } // namespace

    GraphWizGraph TensorToGraph(const Tensor& i_source,
        FormatFlags i_format_flags, size_t i_depth)
    {
        return TensorToGraph(*i_source.GetExpression(), i_format_flags, i_depth);
    }
}
