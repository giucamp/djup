
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <private/common.h>
#include <djup/tensor.h>
#include <limits>

namespace djup
{
    namespace o2o_pattern
    {
        std::vector<Span<const Tensor>> Tokenize(const Tensor& i_tensor);

        std::string TensorSpanToString(Span<const Tensor> i_tensor,
            FormatFlags i_format_flags = FormatFlags::Tidy,
            size_t i_depth = std::numeric_limits<size_t>::max());

    } // namespace o2o_pattern

} // namespace djup
