
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <variant>
#include <vector>
#include <core/hash.h>
#include <private/domain.h>
#include <private/constant_shape.h>
#include <djup/tensor.h>

namespace djup
{
    class Expression;

    class TensorType
    {
    public:

        using Shape = std::variant<std::monostate, ConstantShape, Tensor>;

        TensorType(Domain i_domain, Shape && i_shape);

        Domain GetDomain() const { return m_domain; }

        const Shape & GetShape() const { return m_shape; }

        bool IsUndefinedShape() const { return std::holds_alternative<std::monostate>(m_shape); }

        bool IsFixedShape() const { return std::holds_alternative<ConstantShape>(m_shape); }

        bool IsVariableShape() const { return std::holds_alternative<Tensor>(m_shape); }

        const ConstantShape & GetFixedShape() const { return std::get<ConstantShape>(m_shape); }

        const Tensor & GetVariableShape() const { return std::get<Tensor>(m_shape); }

        Hash GetHash() const noexcept { return m_hash; }

    private:
        Domain m_domain;
        Shape m_shape;
        Hash m_hash;
    };

    template <> struct CharWriter<TensorType>
    {
        void operator() (CharBufferView & i_dest, const TensorType & i_source);
    };

    inline Hash & operator << (Hash & i_dest, const TensorType & i_src)
    {
        return i_dest << i_src.GetHash();
    }
}
