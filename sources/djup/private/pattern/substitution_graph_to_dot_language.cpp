
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/pattern/substitution_graph.h>
#include <private/pattern/discrimination_tree.h>
#include <private/pattern/candidate.h>
#include <private/pattern/utils.h>

namespace djup
{
    namespace pattern
    {
        std::string SubstitutionGraph::ToDotLanguage(std::string_view i_graph_name) const
        {
            const std::string escaped_newline = "\\n";

            StringBuilder dest;

            dest << "digraph G";
            dest.NewLine();
            dest << "{";
            dest.NewLine();
            dest.Tab();

            for (size_t i = 0; i < m_candidate_stack.size(); i++)
            {
                const Candidate& candidate = m_candidate_stack[i];
                dest << "v" << i << "[" << "label = \""
                    << TensorSpanToString(candidate.m_targets, 1)
                    << "\"]";
                    /*<< escaped_newline << TensorSpanToString(candidate.m_discr_edge.m_arguments) 
                    << escaped_newline << "discr_dest:" << candidate.m_discr_edge.m_dest_node << "\"]";*/
            }

            dest.Untab();
            dest << "}";
            dest.NewLine();

            return dest.StealString();
        }

    } // namespace pattern

} // namespace djup
