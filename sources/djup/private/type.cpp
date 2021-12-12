
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/type.h>

namespace djup
{
    Type::Type(Domain i_domain, const FixedShape & i_shape)
        : m_content(TensorType{i_domain, i_shape})
    {

    }

    Type::Type(Domain i_domain, const Tensor & i_shape)
        : m_content(TensorType{i_domain, i_shape})
    {

    }

    void CharWriter<Type>::operator() (CharBufferView & i_dest, const Type & i_source)
    {
        if(i_source.IsTensor())
        {
            const Type::TensorType & tensor_type = i_source.AsTensor();

            i_dest << tensor_type.m_domain;

            if(tensor_type.IsFixedShape())
                i_dest << ' ' << tensor_type.GetFixedShape();
            else if(tensor_type.IsVariableShape())
                i_dest << " [" << tensor_type.GetVariableShape() << ']';
        }
        else if(i_source.IsFunction())
        {
            const Type::FunctionType & function_type = i_source.AsFunction();

            i_dest << function_type.m_return_and_parameter_types.front();
            i_dest << '(';
            i_dest << Span(function_type.m_return_and_parameter_types).subspan(1);
            i_dest << ')';
        }
        else if(i_source.IsTuple())
        {
            i_dest << '{' << Span(i_source.AsTuple().m_element_types) << '}';
        }
        else
        {
            if(!i_source.IsUndefined())
                Error("CharWriter<Type>: Unrecognized type");

            i_dest << "any";
        }
    }
}
