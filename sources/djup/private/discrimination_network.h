
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <memory>
#include <unordered_set>
#include <variant>
#include <private/name.h>
#include <private/expression.h>

namespace djup
{
    class DiscriminationNetwork
    {
    public:

        DiscriminationNetwork();

        void AddPattern(size_t i_pattern_id, const Tensor & i_pattern, const Tensor & i_condition);

    private:

        struct Node
        {
            Expression m_content;
        };

        struct Edge
        {
            
        };

        std::vector<Node> m_nodes;
        std::unordered_multiset<Hash, Edge> m_edges; // the key is a combination of the source and dest node hashes

    private:
        void AddSubPattern(size_t i_node_index, size_t i_pattern_id, const Tensor & i_pattern, const Tensor & i_condition);
    };
}
