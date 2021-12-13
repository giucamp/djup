
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <core/span.h>
#include <core/to_chars.h>
#include <core/hash.h>
#include <core/numeric_cast.h>

namespace djup
{
    class ConstantShape
    {
    public:

        ConstantShape(std::initializer_list<int64_t> i_initializer_list);

        ConstantShape(Span<const int64_t> i_initializer_list);

        int64_t GetRank() const { return static_cast<int64_t>(m_dimensions.size()); }

        int64_t GetLinearSize() const { return m_strides[0]; }

        int64_t GetPhysicalLinearIndex(Span<const int64_t> i_indices) const;

        int64_t GetDimension(int64_t i_index) const { return m_dimensions.at(NumericCast<size_t>(i_index)); }

        int64_t GetDimensionBackward(int64_t i_backward_index) const
            { return m_dimensions.at(NumericCast<size_t>(GetRank() - 1 - i_backward_index)); }

        int64_t GetStride(int64_t i_index) const { return m_strides.at(static_cast<size_t>(i_index)); }

        Span<const int64_t> GetDimensions() const { return m_dimensions; }

        Span<const int64_t> GetStrides() const { return m_strides; }

        bool operator == (const ConstantShape & i_other) const
        {
            return m_dimensions == i_other.m_dimensions;
        }

        bool operator != (const ConstantShape & i_other) const
        {
            return m_dimensions != i_other.m_dimensions;
        }

        static const ConstantShape & Scalar()
        {
            static const ConstantShape scalar({});
            return scalar;
        }

        friend Hash & operator << (Hash & i_dest, const ConstantShape & i_source);

    private:
        std::vector<int64_t> m_dimensions;
        std::vector<int64_t> m_strides;
    };

    template <> struct CharWriter<ConstantShape>
    {
        void operator() (CharBufferView & i_dest, const ConstantShape & i_source) noexcept;
    };

    /* Strides[i] = Product of Dim[j], for i <= j < rank
       Strides[0] = product of all dimensions
       Strides[rank] = 1
       The length of strides is rank + 1 */
    std::vector<int64_t> ComputeStrides(Span<const int64_t> i_dimensions);

    void ComputeStrides(Span<const int64_t> i_dimensions, Span<int64_t> o_strides);

    /* Linearize the indices in i_indices.
        The length of i_strides must be equal to the length of i_dimensions + 1
        The length of i_indices can be greater than the length of i_dimensions, in
        which case broadcasting is applied. If the length of i_indices is less than the
        length of i_dimensions, an error is raised. */
    int64_t GetPhysicalLinearIndex(Span<const int64_t> i_indices,
        Span<const int64_t> i_dimensions,
        Span<const int64_t> i_strides);

    std::optional<ConstantShape> TryBroadcast(Span<const ConstantShape> i_shapes);

    ConstantShape Broadcast(Span<const ConstantShape> i_shapes);

} // namespace liquid
