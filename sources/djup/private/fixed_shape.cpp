
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/fixed_shape.h>

namespace djup
{
    void ValidateDimensions(Span<const int64_t> i_dimensions)
    {
        for(size_t i = 0; i < i_dimensions.size(); i++)
            if(i_dimensions[i] < 0)
                Error("ValidateDimensions - bad shape, ", i, "-th dimension is ", i_dimensions[i]);
    }

    void ValidateStrides(Span<const int64_t> i_stride)
    {
        for (size_t i = 0; i < i_stride.size(); i++)
            if (i_stride[i] < 0)
                Error("ValidateStrides - bad strides, ", i, "-th stride is ", i_stride[i]);
    }

    std::vector<int64_t> ComputeStrides(Span<const int64_t> i_dimensions)
    {
        std::vector<int64_t> strides(i_dimensions.size() + 1);
        ComputeStrides(i_dimensions, strides);
        return strides;
    }

    void ComputeStrides(Span<const int64_t> i_dimensions, Span<int64_t> o_strides)
    {
        ValidateDimensions(i_dimensions);

        if(o_strides.size() != i_dimensions.size() + 1)
            Error("ComputeStrides - bad size of o_strides");

        int64_t product = 1;
        size_t stride_index = i_dimensions.size();
        for (auto dim_it = i_dimensions.rbegin(); dim_it != i_dimensions.rend(); dim_it++)
        {
            o_strides[stride_index] = product;
            stride_index--;
            product *= *dim_it;
        }
        o_strides[0] = product;
    }

    int64_t GetPhysicalLinearIndex(Span<const int64_t> i_indices,
        Span<const int64_t> i_dimensions,
        Span<const int64_t> i_strides)
    {
        ValidateDimensions(i_dimensions);
        ValidateStrides(i_strides);

        if(i_strides.size() != i_dimensions.size() + 1)
            Error("LinearIndex - The rank is ", i_dimensions.size(), ", the length of strides should be ", i_dimensions.size() + 1);

        if (i_indices.size() < i_dimensions.size())
            Error("LinearIndex - Too few indices");

        if (i_indices.size() > i_dimensions.size())
            i_indices = i_indices.subspan(i_indices.size() - i_dimensions.size());

        int64_t linear_index = 0;
        for (size_t i = 0; i < i_indices.size(); i++)
        {
            int64_t const index = i_indices[i];
            int64_t const stride = i_strides[i + 1];
            int64_t const dimension = i_dimensions[i];

            if (index < dimension)
                linear_index += index * stride;
            else if(dimension != 1)
                Error("LinearIndex - the ", i, "-th index is out of bound");
        }
        return linear_index;
    }

    std::optional<FixedShape> TryBroadcast(Span<const FixedShape> i_shapes)
    {
        int64_t rank = 0;
        for (const FixedShape& shape : i_shapes)
            if (shape.GetRank() > rank)
                rank = shape.GetRank();

        std::vector<int64_t> dimensions(static_cast<size_t>(rank), 1);

        for (int64_t dim_index = 0; dim_index < rank; dim_index++)
        {
            int64_t& this_dim = dimensions[static_cast<size_t>(dim_index)];
            for (size_t shape_index = 0; shape_index < i_shapes.size(); shape_index++)
            {
                FixedShape const& shape = i_shapes[shape_index];
                int64_t const rank_offset = rank - shape.GetRank();
                if (dim_index >= rank_offset)
                {
                    int64_t const source_dim = shape.GetDimension(dim_index - rank_offset);
                    if (source_dim != 1)
                    {
                        if (this_dim != 1 && this_dim != source_dim)
                            return {};
                        this_dim = source_dim;
                    }
                }
            }
        }

        return FixedShape{ dimensions };
    }

    FixedShape Broadcast(Span<const FixedShape> i_shapes)
    {
        if(auto result = TryBroadcast(i_shapes))
            return *result;
        Error("Broadcast failure");
    }

    FixedShape::FixedShape(std::initializer_list<int64_t> i_initializer_list)
        : m_dimensions(i_initializer_list), m_strides(i_initializer_list.size() + 1)
    {
        ComputeStrides(m_dimensions, m_strides);
    }

    FixedShape::FixedShape(Span<const int64_t> i_initializer_list)
        : m_dimensions(i_initializer_list.begin(), i_initializer_list.end()),
          m_strides(i_initializer_list.size() + 1)
    {
        ComputeStrides(m_dimensions, m_strides);
    }

    int64_t FixedShape::GetPhysicalLinearIndex(Span<const int64_t> i_indices) const
    {
        return djup::GetPhysicalLinearIndex(i_indices, m_dimensions, m_strides);
    }

    void CharWriter<FixedShape>::operator() (CharBufferView & i_dest, const FixedShape & i_source) noexcept
    {
        i_dest << "[" << i_source.GetDimensions() << "]";
    }

    Hash & operator << (Hash & i_dest, const FixedShape & i_source)
    {
        return i_dest << i_source.m_dimensions;
    }
}
