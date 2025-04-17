
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/pattern/substitution_graph.h>
#include <private/pattern/discrimination_tree.h>
#include <private/pattern/candidate.h>
#include <private/pattern/debug_utils.h>


namespace djup
{
    namespace pattern
    {

        GraphWizGraph SubstitutionGraph::ToDotGraphWiz(std::string_view i_graph_name) const
        {
            GraphWizGraph graph;

            using Handle = Pool<Candidate>::Handle;
            
            struct HandleHash
            {
                std::size_t operator()(const Handle& i_source) const noexcept
                {
                    return i_source.m_index ^ i_source.m_version; // or use boost::hash_combine
                }
            };
            
            std::unordered_map<Handle, uint32_t, HandleHash> handles;

            uint32_t index = 0;
            for (const Candidate& candidate : m_candidates)
            {
                auto label = ToString(
                    candidate.m_source_discr_node, " -> ", candidate.m_edge->m_dest_node
                );
                graph.AddNode(label);
                handles.insert(std::make_pair(m_candidates.HandleOf(candidate), index));

                ++index;
            }

            index = 0;
            for (auto handle_it : handles)
            {
                const Candidate& candidate = m_candidates.GetObject(handle_it.first);

                auto from_it = handles.find(candidate.m_parent_candidate);
                auto to_it = handles.find(handle_it.first);

                if (from_it != handles.end() && to_it != handles.end())
                {
                    graph.AddEdge(from_it->second, to_it->second);
                }
                ++index;
            }

            return graph;
        }

    } // namespace pattern

} // namespace djup
