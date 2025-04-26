
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <private/pattern/substitutions_builder.h>

namespace djup
{
    namespace pattern
    {
        /* Add the source substitutions to the dest vector. If there is
            a contradiction returns false, otherwise true. */
        bool AddSubstitutionsToSolution(
            std::vector<Substitution>& i_dest_substitutions,
            const std::vector<Substitution>& i_source_substitutions)
        {
            const size_t prev_substitution_count = i_dest_substitutions.size();

            // first add all the substitutions
            i_dest_substitutions.insert(i_dest_substitutions.end(),
                i_source_substitutions.begin(), i_source_substitutions.end());

            const size_t new_substitution_count = i_dest_substitutions.size();

            // quadratic search should be fine with a few substitutions
            // for every newly added substitution...
            for (size_t i = prev_substitution_count; i < new_substitution_count; ++i)
            {
                for (size_t j = 0; j < i; ++j)
                {
                    if (i != j && i_dest_substitutions[i].m_identifier_name == i_dest_substitutions[j].m_identifier_name)
                    {
                        if (!AlwaysEqual(i_dest_substitutions[i].m_value, i_dest_substitutions[j].m_value))
                        {
                            return false;
                        }
                    }
                }
            }

            // no contradictions
            return true;
        }

        bool SubstitutionsBuilder::AddSubstitutions(
            const std::vector<Substitution>& i_substitutions,
            uint32_t i_open, uint32_t i_close)
        {
            return AddSubstitutionsToSolution(m_substitutions, i_substitutions);
        }

    } // namespace pattern

} // namespace djup
