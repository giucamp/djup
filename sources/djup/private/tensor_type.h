
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <private/common.h>
#include <private/constant_shape.h>
#include <djup/name.h>
#include <djup/tensor.h>
#include <variant>

namespace djup
{
    class Namespace;

    /** A tensor type has a scalar type, which is a Name, and a shape, which can be:
         - undefined, in which case HasAnyShape() will return false
         - a constant shape, in which case HasConstantShape() will return true
         - a variable shape, that is a non-constant tensor expression, in which 
            case HasVariableShape() will return true. If the shape of the tensor is 
            known, it must have rank equal to 1, otherwise an error is raised. */
    class TensorType
    {
    public:

        using ShapeVector = std::variant<std::monostate, ConstantShape, Tensor>;

        TensorType(Name i_scalar_type = {}, ShapeVector i_shape = {});

        const Name & GetScalarType() const { return m_scalar_type; }

        bool HasAnyShape() const { return !std::holds_alternative<std::monostate>(m_shape); }

        bool HasConstantShape() const { return std::holds_alternative<ConstantShape>(m_shape); }

        bool HasVariableShape() const { return std::holds_alternative<Tensor>(m_shape); }

        const ShapeVector & GetShape() const { return m_shape; }

        const ConstantShape& GetConstantShape() const;

        const Tensor& GetVariableShape() const;

        bool operator == (const TensorType & i_other) const;

        bool operator != (const TensorType & i_other) const { return !operator == (i_other); }

        bool IsSupercaseOf(const TensorType & i_other, const Namespace & i_namespace) const;
    
    private:
        Name m_scalar_type;
        ShapeVector m_shape;
    };

    bool ShapeEqual(const TensorType::ShapeVector& i_first,
        const TensorType::ShapeVector& i_second);
}

namespace core
{
    template <> struct CharWriter<djup::TensorType>
    {
        void operator() (CharBufferView& i_dest, const djup::TensorType& i_source) noexcept;
    };

} // namespace core
