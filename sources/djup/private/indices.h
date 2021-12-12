
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <vector>
#include <private/fixed_shape.h>

namespace djup
{
    class Indices
    {
    public:

        Indices(const FixedShape & i_shape);

        Indices & operator ++ ()
        {
            Increment();
            return *this;
        }

        void operator ++ (int)
        {
            Increment();
        }

        Indices& operator += (int64_t i_addend)
        {
            Add(i_addend);
            return *this;
        }

        explicit operator bool() const
        {
            return IsInBounds();
        }

        template <typename SCALAR_TYPE>
            SCALAR_TYPE & operator[](Span<SCALAR_TYPE> i_scalars)
        {
            int64_t const physical_linear_index = m_shape.GetPhysicalLinearIndex(m_indices);
            return i_scalars[static_cast<size_t>(physical_linear_index)];
        }

        template <typename CONTAINER>
            typename CONTAINER::value_type & operator[](CONTAINER & i_scalars)
        {
            return operator[](Span<typename CONTAINER::value_type>(i_scalars));
        }

        void Increment();

        void Add(int64_t i_addend);

        bool IsInBounds() const;

        int64_t GetLogicalLinearIndex() const { return m_logical_linear_index; }

        const std::vector<int64_t> & GetIndices() const { return m_indices; }

        /*template <typename SCALAR_TYPE>
            const SCALAR_TYPE & At(const TensorValue & i_tensor_value) const
        {
            int64_t const physical_linear_index = static_cast<size_t>(
                i_tensor_value.GetShape().GetPhysicalLinearIndex(m_indices));

            Span<SCALAR_TYPE const> const elements = i_tensor_value.GetAs<SCALAR_TYPE>();
            
            // constant reduction wraps the storage
            size_t const modulo_index = physical_linear_index % elements.size();
            
            return elements.at(modulo_index);
        }

        template <typename SCALAR_TYPE>
            SCALAR_TYPE At(const TensorInitializer & i_initializer) const
        {
            return i_initializer.At<SCALAR_TYPE>(m_indices);
        }*/

    private: 
        void DbgCheck();

    private:
        FixedShape m_shape;
        std::vector<int64_t> m_indices;
        int64_t m_logical_linear_index = 0;
    };

} // namespace liquid
