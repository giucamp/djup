
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/substitutions_tree.h>
#include <limits>

namespace djup
{
    SubstitutionsTree::SubstitutionsTree()
    {
        m_first_free_node = std::numeric_limits<uint32_t>::max();
        Node & root = m_nodes.emplace_back();
        root.m_next_free_node = 1;
        root.m_parent_node = std::numeric_limits<uint32_t>::max();
        root.m_version = 0;
        root.m_dead = 0;
    }

    SubstitutionsTree::NodeHandle SubstitutionsTree::NewNode(NodeHandle i_parent_node)
    {
        if(m_first_free_node == std::numeric_limits<uint32_t>::max())
        {
            m_first_free_node = NumericCast<uint32_t>(m_nodes.size());
            Node & dead_node = m_nodes.emplace_back();
            dead_node.m_next_free_node = std::numeric_limits<uint32_t>::max();
            dead_node.m_dead = 1;
        }

        const uint32_t node_index = m_first_free_node;
        Node & new_node = m_nodes[node_index];
        assert(new_node.m_dead);
        m_first_free_node = new_node.m_next_free_node;
        new_node.m_dead = 0;
        
        new_node.m_parent_node = i_parent_node.m_index;
        Node & parent_node = GetNode(i_parent_node);
        parent_node.m_nested_nodes.push_back(node_index);

        return { node_index, m_nodes[node_index].m_version };
    }

    SubstitutionsTree::Node & SubstitutionsTree::GetNode(NodeHandle i_node)
    {
        assert(IsValid(i_node));
        return m_nodes[i_node.m_index];
    }

    bool SubstitutionsTree::AddSubstitution(NodeHandle i_node, const Name & i_variable_name, Span<VariadicIndex> i_indices, const Tensor & i_value)
    {
        GetNode(i_node).m_substitutions.emplace_back(Substitution{i_variable_name, 
            {i_indices.begin(), i_indices.end()}, i_value});
        return true;
    }

    void SubstitutionsTree::DeleteNode(NodeHandle i_node)
    {
        assert(IsValid(i_node));

        std::vector<uint32_t> nodes_to_delete;
        nodes_to_delete.push_back(i_node.m_index);
        while(!nodes_to_delete.empty())
        {
            const uint32_t node_index = nodes_to_delete.back();
            Node & node = m_nodes[node_index];
            nodes_to_delete.pop_back();
            nodes_to_delete.insert(nodes_to_delete.end(), node.m_nested_nodes.begin(), node.m_nested_nodes.end());

            node.m_next_free_node = m_first_free_node;
            m_first_free_node = node_index;

            node.m_version++;
            node.m_dead = 1;
            node.m_substitutions.clear();
            node.m_nested_nodes.clear();
        }
    }

    bool SubstitutionsTree::IsValid(NodeHandle i_node) const
    {
        const bool valid = i_node.m_version == m_nodes[i_node.m_index].m_version;
        if(valid)
        {
            assert(!m_nodes[i_node.m_index].m_dead);
        }
        return valid;
    }

    SubstitutionsTree::NodeHandle SubstitutionsTree::GetRoot() const
    {
        return {0, m_nodes.front().m_version};
    }

    std::vector<PatternMatch> SubstitutionsTree::GetMathes() const
    {
        std::vector<PatternMatch> matches;
        
        for(const Node & node : m_nodes)
        {
            if(!node.m_dead && node.m_nested_nodes.empty())
                matches.emplace_back();
        }

        return matches;
    }

    void SubstitutionsTree::NodeToString(StringBuilder & i_dest, size_t i_node_index) const
    {
        const Node & node = m_nodes[i_node_index];

        i_dest << "Node " << i_node_index << ": ";
        for(const Substitution & subst : node.m_substitutions)
        {
            i_dest << subst.m_variable_name;
            for(const VariadicIndex & index : subst.m_indices)
                i_dest << '[' << index.m_index << ']';
            i_dest << " = " << ToSimplifiedStringForm(subst.m_value) << ", ";
        }
        i_dest.NewLine();

        i_dest.Tab();
        for(uint32_t nested_index : node.m_nested_nodes)
        {
            NodeToString(i_dest, nested_index);
        }
        i_dest.Untab();
    }

    StringBuilder & operator << (StringBuilder & i_dest, const SubstitutionsTree & i_source)
    {
        i_dest.NewLine();
        i_source.NodeToString(i_dest, 0);
        return i_dest;
    }
}
