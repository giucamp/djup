
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/pattern/substitution_graph.h>
#include <private/pattern/discrimination_tree.h>
#include <private/pattern/candidate.h>

namespace djup
{
    namespace pattern
    {
        void SubstitutionGraph::AddCandidate(uint32_t i_start_node, uint32_t i_end_node,
            CandidateData i_new_candidate_data, std::vector<Substitution>&& i_substitutions)
        {
            assert(i_start_node != i_end_node);

            Candidate candidate;
            candidate.m_data = i_new_candidate_data;
            candidate.m_version = m_next_candidate_version++;
            candidate.m_start_node = i_start_node;
            candidate.m_end_node = i_end_node;

            const uint32_t new_candidate_index = NumericCast<uint32_t>(m_candidate_stack.size());
            CandidateRef candidate_ref{ new_candidate_index, candidate.m_version };

            AddEdge(i_start_node, i_end_node, candidate_ref,
                i_new_candidate_data.m_open, i_new_candidate_data.m_close,
                std::move(i_substitutions));

            m_candidate_stack.push_back(std::move(candidate));
        }

        void SubstitutionGraph::RemoveNode(uint32_t i_node_index)
        {
            std::vector<uint32_t> nodes_to_remove;
            nodes_to_remove.push_back(i_node_index);

            while (!nodes_to_remove.empty())
            {
                uint32_t node = nodes_to_remove.back();
                nodes_to_remove.pop_back();

                // loop all nodes reaching this one
                const auto range = m_edges.equal_range(node);
                for (auto it = range.first; it != range.second;)
                {
                    uint32_t start_node = it->second.m_source_index;

                    if (IsCandidateRefValid(it->second.m_candidate_ref))
                    {
                        Candidate& candidate = m_candidate_stack[it->second.m_candidate_ref.m_index];
                        if (candidate.m_start_node == it->second.m_source_index &&
                            candidate.m_end_node == it->first)
                        {
                            candidate.m_decayed = true;
                        }
                    }

                    assert(m_nodes[it->second.m_source_index].m_outgoing_edges > 0);
                    m_nodes[it->second.m_source_index].m_outgoing_edges--;
                    it = m_edges.erase(it);

                    if (m_nodes[start_node].m_outgoing_edges == 0)
                    {
                        // we have removed the last out-coming edge for i_start_node, we can erase it
                        nodes_to_remove.push_back(start_node);
                    }
                }
            }
        }

        void SubstitutionGraph::AddEdge(uint32_t i_start_node, uint32_t i_end_node,
            CandidateRef i_candidate_ref, uint32_t i_open, uint32_t i_close,
            std::vector<Substitution>&& i_substitutions)
        {
            m_edges.insert({ i_end_node, Edge{i_start_node, i_candidate_ref, i_open, i_close, std::move(i_substitutions)} });
            m_nodes[i_start_node].m_outgoing_edges++;
        }

        void SubstitutionGraph::RemoveEdge(uint32_t i_start_node, uint32_t i_dest_node, CandidateRef i_candidate_ref)
        {
            bool found = false;
            const auto range = m_edges.equal_range(i_dest_node);
            for (auto it = range.first; it != range.second; it++)
            {
                if (it->second.m_source_index == i_start_node &&
                    it->second.m_candidate_ref.m_index == i_candidate_ref.m_index &&
                    it->second.m_candidate_ref.m_version == i_candidate_ref.m_version)
                {
                    assert(m_nodes[i_start_node].m_outgoing_edges > 0);
                    m_nodes[i_start_node].m_outgoing_edges--;
                    m_edges.erase(it);
                    found = true;
                    break;
                }
            }
            assert(found);

            if (m_nodes[i_start_node].m_outgoing_edges == 0)
            {
                // we have removed the last out-coming edge for i_start_node, we can erase it
                RemoveNode(i_start_node);
            }
        }

        uint32_t SubstitutionGraph::NewNode()
        {
            const uint32_t new_node = NumericCast<uint32_t>(m_nodes.size());
            m_nodes.emplace_back();
            return new_node;
        }

    } // namespace pattern

} // namespace djup
