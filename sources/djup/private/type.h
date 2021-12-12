
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <variant>
#include <vector>
#include <core/hash.h>
#include <private/domain.h>
#include <private/fixed_shape.h>
#include <djup/tensor.h>

namespace djup
{
    class Type
    {
    public:

        struct TensorType
        {
            Domain m_domain;
            std::variant<std::monostate, FixedShape, Tensor> m_shape;

            bool IsUndefinedShape() const { return std::holds_alternative<std::monostate>(m_shape); }

            bool IsFixedShape() const { return std::holds_alternative<FixedShape>(m_shape); }

            const FixedShape & GetFixedShape() const { return std::get<FixedShape>(m_shape); }

            bool IsVariableShape() const { return std::holds_alternative<Tensor>(m_shape); }

            const Tensor & GetVariableShape() const { return std::get<Tensor>(m_shape); }
        };

        Type(TensorType && i_tensor_type);

        struct TupleType
        {
            std::vector<Type> m_element_types;
        };

        Type(TupleType && i_tuple_type);

        struct FunctionType
        {
            /** The first element is the function type, the others are the parameter types.
            The reason for not having the return type as separate member is that Type 
            is still an incomplete type here, and it would require some kind of 
            pointer anyway. */
            std::vector<Type> m_return_and_parameter_types;
        };

        Type(FunctionType && i_function_type);

        bool IsUndefined() const { return std::holds_alternative<std::monostate>(m_content); }

        bool IsTensor() const { return std::holds_alternative<TensorType>(m_content); }

        const TensorType & AsTensor() const { return std::get<TensorType>(m_content); }

        bool IsTuple() const { return std::holds_alternative<TupleType>(m_content); }

        const TupleType & AsTuple() const { return std::get<TupleType>(m_content); }

        bool IsFunction() const { return std::holds_alternative<FunctionType>(m_content); }

        const FunctionType & AsFunction() const { return std::get<FunctionType>(m_content); }

        Hash GetHash() const noexcept { return m_hash; }

    private:
        std::variant<std::monostate, TensorType, TupleType, FunctionType> m_content;
        Hash m_hash;
    };

    template <> struct CharWriter<Type>
    {
        void operator() (CharBufferView & i_dest, const Type & i_source);
    };

    Hash & operator << (Hash & i_dest, const Type & i_src)
    {
        return i_dest << i_src.GetHash();
    }
}
