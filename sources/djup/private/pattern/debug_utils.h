
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
    namespace pattern
    {
        /* Warning: this flags will alter the layout of classes, adding
           string where it is useful for debug purpose. They can also 
           enable some Print. */
        #define DJUP_DEBUG_PATTERN_INFO                         true
        #define DJUP_DEBUG_DISCRIMINATION_TREE                  true
        #define DJUP_DEBUG_PATTERN_MATCHING                     true
        #define DJUP_DEBUG_DISABLE_ONE2ONE_PATTERN_MATCHING     false

        /* #if DJUP_DEBUG_PATTERN_INFO
            #pragma message("DJUP_DEBUG_PATTERN_INFO is true")        
        #endif

        #if DJUP_DEBUG_DISCRIMINATION_TREE
            #pragma message("DJUP_DEBUG_DISCRIMINATION_TREE is true")        
        #endif

        #if DJUP_DEBUG_PATTERN_MATCHING
            #pragma message("DJUP_DEBUG_PATTERN_MATCHING is true")        
        #endif

        #if DJUP_DEBUG_DISABLE_ONE2ONE_PATTERN_MATCHING
            #pragma message("DJUP_DEBUG_DISABLE_ONE2ONE_PATTERN_MATCHING is true")        
        #endif */

        std::vector<Span<const Tensor>> Tokenize(const Tensor& i_tensor);

        std::string TensorSpanToString(Span<const Tensor> i_tensor,
            size_t i_depth = std::numeric_limits<size_t>::max(), bool tidy = true);

        void PrintIntVector(const std::vector<uint32_t>&);

        void PrintIntSpan(const Span<uint32_t>&);

    } // namespace pattern

} // namespace djup
