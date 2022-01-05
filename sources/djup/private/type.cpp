
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/type.h>
#include <private/expression.h>
#include <core/hash_variant.h>

namespace djup
{
    TensorType::TensorType()
        : m_domain(Domain::Any)
    {
        m_hash << m_domain;
        m_hash << m_shape;
    }

    TensorType::TensorType(Domain i_domain, Shape && i_shape)
        : m_domain(i_domain), m_shape(std::move(i_shape))
    {
        if(std::holds_alternative<Tensor>(m_shape))
        {
            Tensor & variable_shape = std::get<Tensor>(m_shape);
            if(variable_shape.IsEmpty())
                m_shape = std::monostate{};
            else 
                ; // to do: check the type, convert to constant shape if possible
        }

        m_hash << m_domain;
        m_hash << m_shape;
    }

    bool operator == (const TensorType & i_first, const TensorType & i_second)
    {
        if(i_first.m_hash != i_second.m_hash)
            return false;

        if(i_first.m_domain != i_second.m_domain)
            return false;

        if(i_first.m_shape.index() != i_second.m_shape.index())
            return false;

        if(i_first.IsConstantShape())
        {
            return i_first.GetConstantShape() == i_second.GetConstantShape();
        }
        else if(i_first.IsVariableShape())
        {
            return AlwaysEqual(i_first.GetVariableShape(), i_second.GetVariableShape());
        }
        else
        {
            assert(i_first.IsUndefinedShape());
            return true;
        }
    }

    bool TypeMatches(const TensorType & i_target, const TensorType & i_pattern)
    {
        if(!IsSupersetOf(i_target.GetDomain(), i_pattern.GetDomain()))
            return false;

        if(i_pattern.IsUndefinedShape())
            return true;

        if(i_pattern.IsConstantShape())
            return i_target.IsConstantShape() && i_target.GetConstantShape() == i_pattern.GetConstantShape();
        
        assert(i_pattern.IsVariableShape());

        assert(false); // to do: should use pattern matching here
        return false;
    }

    bool operator != (const TensorType & i_first, const TensorType & i_second)
    {
        return !(i_first == i_second);
    }

    void CharWriter<TensorType>::operator() (CharBufferView & i_dest, const TensorType & i_source)
    {
        i_dest << i_source.GetDomain();

        if(i_source.IsConstantShape())
            i_dest << ' ' << i_source.GetConstantShape();
        else if(i_source.IsVariableShape())
            i_dest << " [" << i_source.GetVariableShape() << ']';
    }
}
