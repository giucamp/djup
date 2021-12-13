
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/type.h>
#include <private/expression.h>
#include <core/hash_variant.h>

namespace djup
{
    Hash & operator << (Hash & i_dest, const Type::TensorType & i_src)
    {
        i_dest << i_src.m_domain;
        i_dest << i_src.m_shape;
        return i_dest;
    }

    Hash & operator << (Hash & i_dest, const Type::TupleType & i_src)
    {
        i_dest << i_src.m_element_types;
        return i_dest;
    }

    Hash & operator << (Hash & i_dest, const Type::FunctionType & i_src)
    {
        i_dest << i_src.m_return_and_parameter_types;
        return i_dest;
    }

    Type::TensorType::TensorType(Domain i_domain, Shape && i_shape)
        : m_domain(i_domain), m_shape(std::move(i_shape))
    {
        assert(false); // to do: detect constant shapes
    }

    Type::Type(TensorType && i_tensor_type)
        : m_content(std::move(i_tensor_type))
    {
        m_hash << m_content;
    }

    Type::Type(TupleType && i_tuple_type)
        : m_content(std::move(i_tuple_type))
    {
        m_hash << m_content;
    }

    Type::Type(FunctionType && i_function_type)
        : m_content(std::move(i_function_type))
    {
        m_hash << m_content;
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
