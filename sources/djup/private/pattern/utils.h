
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <djup/tensor.h>
#include <limits>

namespace djup
{
    namespace pattern
    {
        std::vector<Span<const Tensor>> Tokenize(const Tensor& i_tensor);

        std::string TensorSpanToString(Span<const Tensor> i_tensor,
            size_t i_depth = std::numeric_limits<size_t>::max(), bool tidy = true);
        
        void PrintIntVector(const std::vector<uint32_t>&);
        void PrintIntSpan(const Span<uint32_t>&);


    } // namespace pattern

} // namespace djup
