
#pragma once
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

    void CharWriter<Type>::operator() (CharBufferView & i_dest, const Type & i_source) noexcept
    {
        
    }
}
