
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/pattern/substitution_graph.h>
#include <private/pattern/discrimination_tree.h>
#include <private/pattern/candidate.h>
#include <private/builtin_names.h>
#include <core/flags.h>
#include <core/to_string.h>

namespace djup
{
    namespace pattern
    {
        SubstitutionGraph::SubstitutionGraph(const DiscriminationTree & m_discrimination_net)
            : m_discrimination_net(m_discrimination_net)
        {
        }

        SubstitutionGraph::~SubstitutionGraph() = default;

        bool SubstitutionGraph::IsCandidateRefValid(CandidateRef i_ref) const
        {
            return i_ref.m_index < m_candidate_stack.size() && i_ref.m_version == m_candidate_stack[i_ref.m_index].m_version;
        }

    } // namespace pattern

} // namespace djup
