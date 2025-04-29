
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
            GraphWizGraph & m_graph;
            int32_t m_node_count{};
        };

        size_t TensorToGraph(ExpressionToGraphContext & i_context,
            const Expression& i_source, FormatFlags i_format_flags, size_t i_depth)
        {
            (void)i_format_flags;

            size_t vertex_index;
            
            if (i_source.GetMetadata().m_is_literal)
            {
                const std::string literal = i_source.GetName().AsString();
                vertex_index = i_context.m_graph.GetNodeCount();
                i_context.m_graph.AddNode(std::move(literal));
            }
            else
            {
                std::string name = i_source.GetName().AsString();
                if (!i_source.GetType().IsEmpty())
                    name = ToString(i_source.GetType()) + " " + name;

                vertex_index = i_context.m_graph.GetNodeCount();
                i_context.m_graph.AddNode(std::move(name));

                if (i_depth > 0)
                {
                    for (const Tensor& argument : i_source.GetArguments())
                    {
                        size_t arg_vertex = TensorToGraph(
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
            const std::string & i_label,
            FormatFlags i_format_flags, size_t i_depth)
        {
            GraphWizGraph graph(i_label);
            ExpressionToGraphContext context{ graph, 0 };
            TensorToGraph(context, i_source, i_format_flags, i_depth);
            return context.m_graph;
        }

    } // namespace

    GraphWizGraph TensorToGraph(const Tensor& i_source,
        FormatFlags i_format_flags, size_t i_depth)
    {
        return TensorToGraph(*i_source.GetExpression(),
            ToSimplifiedString(i_source), i_format_flags, i_depth);
    }
}
