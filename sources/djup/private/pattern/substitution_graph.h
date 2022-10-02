
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <private/expression.h>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>
#include <limits>

namespace djup
{
    namespace pattern
    {

        class SubstitutionGraph
        {

        public:



        private:

            struct Node;
            struct Candidate;
            struct Substitution;
            struct CandidateRef;
            struct Edge;

        private:


        private:

            std::vector<Candidate> m_candidates;
            std::vector<Node> m_graph_nodes;
            std::unordered_multimap<uint32_t, Edge> m_edges; // the key is the destination node
            uint32_t m_next_candidate_version{};
            #if DBG_CREATE_GRAPHVIZ_SVG
                std::string m_graph_name;
            #endif
        };
    
    } // namespace pattern

} // namespace djup
