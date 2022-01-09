
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <djup/tensor.h>
#include <private/expression.h>
#include <core/to_string.h>

namespace djup
{
    namespace
    {
        void TensorToString(StringBuilder & i_dest, const Tensor & i_source)
        {
            i_dest << i_source.GetExpression()->GetName();
        }
    }

    void ToSimplifiedStringForm(StringBuilder & i_dest, const Tensor & i_source)
    {
        if(!i_source.IsEmpty())
        {
            const Expression & expr = *i_source.GetExpression();

            if(expr.IsVariable())
            {
                size_t size = i_dest.GetSize();
                i_dest << expr.GetType();
                if(size != i_dest.GetSize())
                    i_dest << ' ';
            }

            i_dest << expr.GetName();

            auto arguments = Span(expr.GetArguments());
            if(!arguments.empty())
            {
                i_dest << '(';

                for (size_t i = 0; i < arguments.size(); i++)
                {
                    if(i != 0)
                        i_dest << ", ";
                    ToSimplifiedStringForm(i_dest, arguments[i]);
                }

                i_dest << ')';
            }
        }
    }

    StringBuilder & operator << (StringBuilder & i_dest, const Tensor & i_source)
    {
        TensorToString(i_dest, i_source);
        return i_dest;
    }
}
