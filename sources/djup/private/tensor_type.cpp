
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <private/tensor_type.h>
#include <private/namespace.h>

namespace djup
{
    TensorType::TensorType(Name i_scalar_type, ShapeVector i_shape)
        : m_scalar_type(std::move(i_scalar_type)), m_shape(std::move(i_shape))
    {
        // if the shape is an empyty tensor set the variant to the monostate
        if (HasVariableShape())
        {
            const Tensor shape = GetVariableShape();
            if (djup::IsEmpty(shape))
                m_shape = {};
        }

        /*if (HasVariableShape() && IsConstant(GetVariableShape()))
        {
            // the shape is not really variable
            const Tensor& shape_vector = GetVariableShape();
            const TensorType& type_of_shape = shape_vector.GetExpression()->GetType();
            if (type_of_shape.GetConstantShape().GetRank() != 1)
                Error("The shape of a tensor must be a vector, this tank is ", 
                    type_of_shape.GetConstantShape().GetRank());

            if (type_of_shape.GetScalarType() != ScalarType::Integer)
                Error("The shape vector must be integral, scalar type is ", type_of_shape.GetScalarType());

            auto dimensions = ConstantToVector<Integer>(shape_vector);
            m_shape = ConstantShape(dimensions);
        }*/
    }

    const ConstantShape& TensorType::GetConstantShape() const
    {
        if (!HasConstantShape())
            Error("TensorType::GetConstantShape - ", *this, ": not a fixed shape");
        return std::get<ConstantShape>(m_shape);
    }

    const Tensor& TensorType::GetVariableShape() const
    {
        if (!HasVariableShape())
            Error("TensorType::GetConstantShape - ", *this, ": not a variable shape");
        return std::get<Tensor>(m_shape);
    }

    bool TensorType::operator == (const TensorType& i_other) const
    {
        if (m_scalar_type != i_other.m_scalar_type)
            return false;

        if (m_shape.index() != i_other.m_shape.index())
            return false;

        struct Visitor
        {
            const TensorType & m_this;
            const TensorType & m_other;

            bool operator () (std::monostate) const
            {
                return !m_other.HasAnyShape();
            }

            bool operator () (const ConstantShape& i_shape) const
            {
                if (m_other.HasConstantShape())
                    return m_this.GetConstantShape() == m_other.GetConstantShape();
                return false;
            }

            bool operator () (const Tensor& i_shape) const
            {
                return AlwaysEqual(m_this.GetVariableShape(), m_other.GetVariableShape());
            }
        };

        return std::visit(Visitor{ *this, i_other }, m_shape);
    }

    bool ShapeEqual(const TensorType::ShapeVector& i_first,
        const TensorType::ShapeVector& i_second)
    {
        if (i_first.index() != i_second.index())
            return false;

        if (std::holds_alternative<ConstantShape>(i_first))
            return std::get<ConstantShape>(i_first) == std::get<ConstantShape>(i_second);

        if (std::holds_alternative<Tensor>(i_first))
            return AlwaysEqual(
                std::get<Tensor>(i_first),
                std::get<Tensor>(i_second));

        return true;
    }
}

namespace core
{
    // CharWriter for TensorType
    void CharWriter<djup::TensorType>::operator() (CharBufferView& i_dest, const djup::TensorType& i_source) noexcept
    {
        using namespace djup;

        i_dest << i_source.GetScalarType();
        if (!i_source.GetScalarType().IsEmpty() &&
            i_source.HasAnyShape())
        {
            i_dest << " ";
        }

        struct Visitor
        {
            CharBufferView& m_dest;
            const TensorType& m_source;

            void operator () (std::monostate) const
            {
                
            }

            void operator () (const ConstantShape& i_shape) const
            {
                m_dest << i_shape;
            }

            void operator () (const Tensor& i_shape) const
            {
                m_dest << ToSimplifiedString(i_shape);
            }
        };

        std::visit(Visitor{ i_dest, i_source }, i_source.GetShape());

    }

} // namespace core
