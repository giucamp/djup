
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <djup/tensor.h>
#include <private/expression.h>
#include <private/builtin_names.h>
#include <core/to_string.h>

namespace djup
{
    namespace
    {
        void TensorToString(StringBuilder & i_dest, const Tensor & i_source)
        {
            i_dest << i_source.GetExpression()->GetName();
        }

        void TensorSpanToString(StringBuilder& i_dest, Span<const Tensor> i_tensors)
        {
            i_dest << '(';

            for (size_t i = 0; i < i_tensors.size(); i++)
            {
                if (i != 0)
                    i_dest << ", ";
                ToSimplifiedStringForm(i_dest, i_tensors[i]);
            }

            i_dest << ')';
        }
    }

    void ToSimplifiedStringForm(StringBuilder& i_dest, const Expression & i_source)
    {
        if (i_source.GetName() == builtin_names::Identifier)
        {
            const Expression& first_arg = *i_source.GetArgument(0).GetExpression();
            const Expression& second_arg = *i_source.GetArgument(1).GetExpression();
            assert(first_arg.GetName() == builtin_names::TensorType);

            i_dest << first_arg.GetArgument(0).GetExpression()->GetName();
            i_dest << ' ';
            i_dest << second_arg.GetName();
        }
        else if (i_source.GetName() == builtin_names::Literal)
        {
            ToSimplifiedStringForm(i_dest, i_source.GetArgument(0));
        }
        else if (i_source.GetName() == builtin_names::RepetitionsZeroToMany)
        {
            if (i_source.GetArguments().size() == 1)
                ToSimplifiedStringForm(i_dest, i_source.GetArgument(0));
            else
                TensorSpanToString(i_dest, i_source.GetArguments());
            i_dest << "...";
        }
        else if (i_source.GetName() == builtin_names::RepetitionsOneToMany)
        {
            ToSimplifiedStringForm(i_dest, i_source.GetArgument(0));
            i_dest << "..";
        }
        else if (i_source.GetName() == builtin_names::RepetitionsZeroToOne)
        {
            ToSimplifiedStringForm(i_dest, i_source.GetArgument(0));
            i_dest << "?";
        }
        else
        {
            i_dest << i_source.GetName();

            auto arguments = Span(i_source.GetArguments());
            if (!arguments.empty())
            {
                TensorSpanToString(i_dest, arguments);
            }
        }
    }

    std::string ToSimplifiedStringForm(const Expression& i_source)
    {
        StringBuilder dest;
        ToSimplifiedStringForm(dest, i_source);
        return dest.StealString();
    }

    void ToSimplifiedStringForm(StringBuilder & i_dest, const Tensor & i_source)
    {
        if(!IsEmpty(i_source))
        {
            const Expression & expr = *i_source.GetExpression();
            ToSimplifiedStringForm(i_dest, expr);
        }
    }

    StringBuilder & operator << (StringBuilder & i_dest, const Tensor & i_source)
    {
        TensorToString(i_dest, i_source);
        return i_dest;
    }
}
