
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <djup/tensor.h>
#include <private/expression.h>
#include <private/builtin_names.h>
#include <core/to_string.h>
#include <core/flags.h>

namespace djup
{
    namespace
    {
        void TensorSpanToString(StringBuilder& i_dest, Span<const Tensor> i_tensors,
            FormatFlags i_format_flags = FormatFlags::Tidy,
            size_t i_depth = std::numeric_limits<size_t>::max())
        {
            i_dest << '(';
            for (size_t i = 0; i < i_tensors.size(); i++)
            {
                if (i != 0)
                    i_dest << ", ";
                ToSimplifiedString(i_dest, i_tensors[i], i_format_flags, i_depth);
            }
            i_dest << ')';
        }
    }

    void ToSimplifiedString(StringBuilder& i_dest, const Expression& i_source,
        FormatFlags i_format_flags, size_t i_depth)
    {
        if (HasFlag(i_format_flags, FormatFlags::Tidy))
        {
            if (i_source.GetMetadata().m_is_literal)
            {
                i_dest << i_source.GetName().AsString();
            }
            else if (i_source.GetName() == builtin_names::RepetitionsZeroToMany)
            {
                if (i_source.GetArguments().size() == 1)
                    ToSimplifiedString(i_dest, i_source.GetArgument(0), FormatFlags::Tidy, i_depth);
                else
                    TensorSpanToString(i_dest, i_source.GetArguments(), i_format_flags, i_depth);
                i_dest << "...";
            }
            else if (i_source.GetName() == builtin_names::RepetitionsOneToMany)
            {
                ToSimplifiedString(i_dest, i_source.GetArgument(0), i_format_flags, i_depth);
                i_dest << "..";
            }
            else if (i_source.GetName() == builtin_names::RepetitionsZeroToOne)
            {
                ToSimplifiedString(i_dest, i_source.GetArgument(0), i_format_flags, i_depth);
                i_dest << "?";
            }
            else if (i_source.GetName() == builtin_names::Tuple)
            {
                TensorSpanToString(i_dest, i_source.GetArguments(), i_format_flags, i_depth);
            }
            else
            {
                if (i_source.GetType() != TensorType{})
                {
                    i_dest << i_source.GetType();
                    if (!i_source.GetName().IsEmpty())
                        i_dest << " ";
                }
                i_dest << i_source.GetName();

                if (i_depth > 1)
                {
                    auto arguments = Span(i_source.GetArguments());
                    if (!arguments.empty())
                    {
                        TensorSpanToString(i_dest, arguments, i_format_flags, i_depth - 1);
                    }
                }
            }
        }
        else
        {
            i_dest << i_source.GetName();

            if (i_depth > 1)
            {
                auto arguments = Span(i_source.GetArguments());
                if (!arguments.empty())
                {
                    TensorSpanToString(i_dest, arguments, {}, i_depth - 1);
                }
            }
        }
    }

    void ToSimplifiedString(StringBuilder & i_dest, const Tensor & i_source, FormatFlags i_format_flags, size_t i_depth)
    {
        if(!IsEmpty(i_source))
        {
            const Expression & expr = *i_source.GetExpression();
            ToSimplifiedString(i_dest, expr, i_format_flags, i_depth);
        }
    }
}
