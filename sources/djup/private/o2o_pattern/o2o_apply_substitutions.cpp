
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <private/namespace.h>
#include <private/substitute_by_predicate.h>
#include <private/o2o_pattern/o2o_pattern_match.h>

namespace djup
{
    namespace o2o_pattern
    {
        namespace
        {
            struct ApplySubstitutionContext
            {
                std::unordered_map<std::shared_ptr<const Expression>,
                    std::shared_ptr<const Expression> > m_processed;

                uint32_t m_curr_depth{ 0 };
            };

            Tensor ApplySubstitutionsImpl(const Tensor & i_where,
                Span<const Substitution> i_substitutions,
                ApplySubstitutionContext & i_context)
            {
                auto const it = i_context.m_processed.find(i_where.GetExpression());
                if (it != i_context.m_processed.end())
                    return Tensor(it->second);

                std::vector<Tensor> new_arguments;

                bool some_argument_replaced = false;
                const auto & arguments = i_where.GetExpression()->GetArguments();
                for (size_t i = 0; i < arguments.size(); ++i)
                {
                    const Tensor & argument = arguments[i];

                    IntInterval cardinality = GetCardinality(argument);
                    if (cardinality != IntInterval{ 1, 1 })
                    {
                        ++i_context.m_curr_depth;

                        Tensor replacement = ApplySubstitutionsImpl(argument, i_substitutions, i_context);

                        DJUP_ASSERT(i_context.m_curr_depth != 0);
                        --i_context.m_curr_depth;
                    }
                    else
                    {
                        for (const Substitution & subst : i_substitutions)
                        {
                            if (argument.GetExpression()->GetName() == subst.m_identifier_name)
                            {
                                new_arguments.push_back(ApplySubstitutionsImpl(subst.m_value, i_substitutions, i_context));
                                some_argument_replaced = true;
                                break;
                            }
                        }
                        if (!some_argument_replaced)
                            new_arguments.push_back(ApplySubstitutionsImpl(argument, i_substitutions, i_context));
                    }

                    some_argument_replaced = some_argument_replaced ||
                        new_arguments.back().GetExpression() != argument.GetExpression();
                }
                if (some_argument_replaced)
                {
                    const Expression & expr = *i_where.GetExpression();
                    return MakeExpression(
                        expr.GetType(), expr.GetName(), new_arguments, expr.GetMetadata());
                }
                else
                    return i_where;
            }

        } // unnamed namespace

        Tensor ApplySubstitutions(const Tensor & i_where,
            Span<const Substitution> i_substitutions)
        {
            /*ApplySubstitutionContext context;
            return ApplySubstitutionsImpl(i_where, i_substitutions, context); */

            return SubstituteByPredicate(i_where, [i_substitutions](const Tensor i_tensor) {
                for (const Substitution & subst : i_substitutions)
                {
                    if (i_tensor.GetExpression()->GetName() == subst.m_identifier_name)
                    {
                        return subst.m_value;
                    }
                }
                return i_tensor;
            });
        }
        
    } // namespace o2o_pattern

} // namespace djup

