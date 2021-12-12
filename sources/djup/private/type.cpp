
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

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
