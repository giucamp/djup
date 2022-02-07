
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <djup/tensor.h>
#include <private/name.h>
#include <private/expression.h>
#include <private/pattern_match.h>
#include <core/to_string.h>
#include <vector>

namespace djup
{
    struct VariadicIndex
    {
        uint32_t m_index, m_count;
    };

    class SubstitutionsTree
    {
    public:

        struct NodeHandle
        {
            uint32_t m_index;
            uint32_t m_version;
        };

        SubstitutionsTree();

        NodeHandle NewNode(NodeHandle i_parent_node);

        bool AddSubstitution(NodeHandle i_node, const Name & i_variable_name, Span<VariadicIndex> i_indices, const Tensor & i_value);

        void DeleteNode(NodeHandle i_node);

        bool IsValid(NodeHandle i_node) const;

        NodeHandle GetRoot() const;

        std::vector<PatternMatch> GetMathes() const;

        friend StringBuilder & operator << (StringBuilder & i_dest, const SubstitutionsTree & i_source);

    private:

        struct Substitution
        {
            Name m_variable_name;
            std::vector<VariadicIndex> m_indices;
            Tensor m_value;
        };

        struct Node
        {
            uint32_t m_version : 31;
            uint32_t m_dead : 1;
            uint32_t m_parent_node{};
            uint32_t m_next_free_node{};
            std::vector<Substitution> m_substitutions;
            std::vector<uint32_t> m_nested_nodes;
        };

        void NodeToString(StringBuilder & i_dest, size_t i_node_index) const;

        Node & GetNode(NodeHandle i_node);

    private:
        std::vector<Node> m_nodes;
        uint32_t m_first_free_node{};
    };
}
