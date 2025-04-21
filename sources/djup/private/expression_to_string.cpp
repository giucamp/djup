
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <djup/tensor.h>
#include <djup/expression.h>
#include <private/builtin_names.h>
#include <core/to_string.h>
#include <core/flags.h>

namespace djup
{
    namespace
    {
        void TensorSpanToString(StringBuilder& i_dest, Span<const Tensor> i_tensors, size_t i_depth, FormatFlags i_format_flags = FormatFlags::Tidy)
        {
            i_dest << '(';
            for (size_t i = 0; i < i_tensors.size(); i++)
            {
                if (i != 0)
                    i_dest << ", ";
                ToSimplifiedStringForm(i_dest, i_tensors[i], i_depth, i_format_flags);
            }
            i_dest << ')';
        }
    }

    void ToSimplifiedStringForm(StringBuilder& i_dest, const Expression& i_source,
        size_t i_depth, FormatFlags i_format_flags)
    {
        if (HasFlag(i_format_flags, FormatFlags::Tidy))
        {
            if (i_source.GetName() == builtin_names::Identifier)
            {
                const Expression& first_arg = *i_source.GetArgument(0).GetExpression();
                const Expression& second_arg = *i_source.GetArgument(1).GetExpression();
                DJUP_ASSERT(first_arg.GetName() == builtin_names::TensorType);

                i_dest << first_arg.GetArgument(0).GetExpression()->GetName();
                if (!second_arg.GetName().IsEmpty())
                {
                    i_dest << ' ';
                    i_dest << second_arg.GetName();
                }
            }
            else if (i_source.GetName() == builtin_names::Literal)
            {
                i_dest << i_source.GetArgument(0).GetExpression()->GetName().AsString();
            }
            else if (i_source.GetName() == builtin_names::RepetitionsZeroToMany)
            {
                if (i_source.GetArguments().size() == 1)
                    ToSimplifiedStringForm(i_dest, i_source.GetArgument(0), i_depth);
                else
                    TensorSpanToString(i_dest, i_source.GetArguments(), i_depth, i_format_flags);
                i_dest << "...";
            }
            else if (i_source.GetName() == builtin_names::RepetitionsOneToMany)
            {
                ToSimplifiedStringForm(i_dest, i_source.GetArgument(0), i_depth, i_format_flags);
                i_dest << "..";
            }
            else if (i_source.GetName() == builtin_names::RepetitionsZeroToOne)
            {
                ToSimplifiedStringForm(i_dest, i_source.GetArgument(0), i_depth, i_format_flags);
                i_dest << "?";
            }
            else
            {
                i_dest << i_source.GetName();

                if (i_depth > 1)
                {
                    auto arguments = Span(i_source.GetArguments());
                    if (!arguments.empty())
                    {
                        TensorSpanToString(i_dest, arguments, i_depth - 1, i_format_flags);
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
                    TensorSpanToString(i_dest, arguments, i_depth - 1, {});
                }
            }
        }
    }

    std::string ToSimplifiedStringForm(const Expression& i_source, size_t i_depth, FormatFlags i_format_flags)
    {
        StringBuilder dest;
        ToSimplifiedStringForm(dest, i_source, i_depth, i_format_flags);
        return dest.StealString();
    }

    void ToSimplifiedStringForm(StringBuilder & i_dest, const Tensor & i_source, size_t i_depth, FormatFlags i_format_flags)
    {
        if(!IsEmpty(i_source))
        {
            const Expression & expr = *i_source.GetExpression();
            ToSimplifiedStringForm(i_dest, expr, i_depth, i_format_flags);
        }
    }

    /*StringBuilder & operator << (StringBuilder & i_dest, const Tensor & i_source)
    {
        TensorToString(i_dest, i_source, FormatFlags::Tidy);
        return i_dest;
    }*/
}
