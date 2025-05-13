//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <private/m2o_pattern/m2o_substitution_graph.h>
#include <private/m2o_pattern/m2o_discrimination_tree.h>
#include <private/m2o_pattern/m2o_pattern_info.h>
#include <private/namespace.h>
#include <private/substitute_by_predicate.h>
#include <private/builtin_names.h>
#include <algorithm>

namespace djup
{
    namespace m2o_pattern
    {
        SubstitutionGraph::SubstitutionGraph(const DiscriminationTree& i_discrimination_net,
                SolutionType i_solution_type)
            : m_discrimination_tree(i_discrimination_net),
              m_solution_type(i_solution_type)
        {
        }

        SubstitutionGraph::~SubstitutionGraph() = default;

        void SubstitutionGraph::FindMatches(
            const Namespace& i_namespace, const Tensor& i_target,
            std::function<void()> i_step_callback)
        {
        }

        Range GetUsableCount(const ArgumentInfo & i_argument_info,
            uint32_t i_target_remaining_targets, uint32_t i_pattern_size)
        {
            // number of total parameters usable for the repeated pattern
            Range usable;

            DJUP_ASSERT(i_target_remaining_targets >= static_cast<uint32_t>(
                i_argument_info.m_remaining.m_min) );
            usable.m_max = i_target_remaining_targets - i_argument_info.m_remaining.m_min;
            if (i_argument_info.m_remaining.m_max == Range::s_infinite)
            {
                usable.m_min = 0;
            }
            else
            {
                DJUP_ASSERT(i_target_remaining_targets >= static_cast<uint32_t>(
                    i_argument_info.m_remaining.m_max));
                usable.m_min = i_target_remaining_targets - i_argument_info.m_remaining.m_max;
            }

            usable = i_argument_info.m_cardinality.ClampRange(usable);

            /* align the usable range to be a multiple of sub_pattern_count.
               usable.m_min is aligned to the highest multiple, while usable.m_max
               to the lowest. */
            usable.m_min += i_pattern_size - 1;
            usable.m_min -= usable.m_min % i_pattern_size;
            usable.m_max -= usable.m_max % i_pattern_size;

            return usable;
        }

        
        Tensor ApplySubstitutions(const Tensor& i_where,
            Span<const Substitution> i_substitutions)
        {
            return SubstituteByPredicate(i_where, [i_substitutions](const Tensor i_tensor) {
                for (const Substitution & subst : i_substitutions)
                {
                    /*if (!i_tensor.GetExpression()->GetType().IsSupercaseOf(
                        subst.m_value.GetExpression()->GetType(), )
                    {
                        Error();
                    } */

                    if (//IsIdentifier(i_tensor) &&
                        i_tensor.GetExpression()->GetName() == subst.m_identifier_name)
                    {
                        return subst.m_value;
                    }
                }
                return i_tensor;
            });
        }

    } // namespace m2o_pattern

} // namespace djup
