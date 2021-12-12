
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

        // tensor types
        Type(Domain i_domain, const FixedShape & i_shape);
        Type(Domain i_domain, const Tensor & i_shape);

        struct TensorType
        {
            Domain m_domain;
            std::variant<std::monostate, FixedShape, Tensor> m_shape;
        };

        struct TupleType
        {
            std::vector<Type> m_element_types;
        };

        struct FunctionType
        {
            std::vector<Type> m_return_and_parameter_types;
        };

        bool IsTensor() const { return std::holds_alternative<TensorType>(m_content); }

        const TensorType & AsTensor() const { return std::get<TensorType>(m_content); }

        bool IsTuple() const { return std::holds_alternative<TupleType>(m_content); }

        const TupleType & AsTuple() const { return std::get<TupleType>(m_content); }

        bool IsFunction() const { return std::holds_alternative<FunctionType>(m_content); }

        const FunctionType & AsFunction() const { return std::get<FunctionType>(m_content); }

    private:
        std::variant<std::monostate, TensorType, TupleType, FunctionType> m_content;
    };

    template <> struct CharWriter<Type>
    {
        void operator() (CharBufferView & i_dest, const Type & i_source) noexcept;
    };
}
