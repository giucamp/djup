
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/type.h>
#include <private/expression.h>
#include <core/hash_variant.h>

namespace djup
{
    TensorType::TensorType(Domain i_domain, Shape && i_shape)
        : m_domain(i_domain), m_shape(std::move(i_shape))
    {
        m_hash << m_domain;
        m_hash << m_shape;

        assert(false); // to do: detect constant shapes
    }

    void CharWriter<TensorType>::operator() (CharBufferView & i_dest, const TensorType & i_source)
    {
        i_dest << i_source.GetDomain();

        if(i_source.IsFixedShape())
            i_dest << ' ' << i_source.GetFixedShape();
        else if(i_source.IsVariableShape())
            i_dest << " [" << i_source.GetVariableShape() << ']';
    }

    bool IsTypeExpression(const Expression & i_expr)
    {
        if(i_expr.IsTensorExpr())
        {
            const Expression::TensorExpr & tensor_expr = i_expr.AsTensorExpr();
            if(tensor_expr.m_name.IsEmpty())
                return true;
        }
        return false;
    }
}
