
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/discrimination_network.h>

namespace djup
{
    namespace
    {
        Hash CombineHashes(Hash i_first, Hash i_second)
        {
            i_first << i_second;
            return i_first;
        }
    }

    DiscriminationNetwork::DiscriminationNetwork()
    {

    }

    void DiscriminationNetwork::AddPattern(size_t i_pattern_id, 
        const Tensor & i_pattern, const Tensor & i_condition)
    {
        AddSubPattern(0, i_pattern_id, i_pattern, i_condition);
    }

    void DiscriminationNetwork::AddSubPattern(size_t i_node_index, size_t i_pattern_id, 
        const Tensor & i_pattern, const Tensor & i_condition)
    {
        //i_pattern.GetExpression()->
    }
}
