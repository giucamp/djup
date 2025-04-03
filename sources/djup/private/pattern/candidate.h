
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <private/expression.h>
#include <private/pattern/pattern_info.h>
#include <private/pattern/discrimination_tree.h>

namespace djup
{
    namespace pattern
    {
        struct Substitution
        {
            Name m_variable_name;
            Tensor m_value;
        };

        struct Candidate
        {
            /** index of ancestor candidate node, or ~ for the root */
            uint32_t m_parent_candidate{};

            /** data related to the discrimination tree.The discrimination 
                during a pattern matching . */
            DiscriminationTree::Edge m_discr_edge;
            uint16_t m_pattern_offset{};
            uint32_t m_repetitions_offset{};
            uint32_t m_repetitions{1};
            //uint32_t m_discr_node_index{};

            /* data related to the target (the expression patterns are matched against) */
            Span<const Tensor> m_targets;
            uint16_t m_target_offset{};

            uint32_t m_open{};
            uint32_t m_close{};

            uint32_t m_outcoming_edges{};

            std::vector<Substitution> m_substitutions;

            bool AddSubstitution(const Name& i_variable_name, const Tensor& i_value)
            {
                m_substitutions.emplace_back(Substitution{ i_variable_name, i_value });
                return true;
            }
        };
    
    } // namespace pattern

} // namespace djup
