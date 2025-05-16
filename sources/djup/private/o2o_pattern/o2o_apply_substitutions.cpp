
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <private/namespace.h>
#include <private/builtin_names.h>
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
                Span<const Substitution> m_substitutions;

                std::unordered_map<std::shared_ptr<const Expression>,
                    std::shared_ptr<const Expression> > m_processed;

                uint32_t m_curr_depth{ 0 };
            };

            void GetInvolvedIdentifiers(std::vector<std::shared_ptr<const Expression>> & o_involved, const Tensor & i_expr)
            {
                if (IsIdentifier(i_expr) && !IsRepetition(i_expr))
                {
                    o_involved.push_back(i_expr.GetExpression());
                }

                for (const Tensor & arg : i_expr.GetExpression()->GetArguments())
                {
                    GetInvolvedIdentifiers(o_involved, arg);
                }
            }

            Tensor SubstituteSingle(const Tensor & i_tensor, const ApplySubstitutionContext & i_context)
            {
                for (const Substitution & subst : i_context.m_substitutions)
                {
                    if (i_tensor.GetExpression()->GetName() == subst.m_identifier_name)
                    {
                        return subst.m_value;
                    }
                }
                return i_tensor;
            }

            Tensor ApplySubstitutionsImpl(const Tensor & i_where,
                ApplySubstitutionContext & i_context)
            {
                auto const it = i_context.m_processed.find(i_where.GetExpression());
                if (it != i_context.m_processed.end())
                    return Tensor(it->second);

                Tensor replacement = SubstituteSingle(i_where, i_context);

                std::vector<Tensor> new_arguments;

                bool some_argument_replaced = false;
                const auto & arguments = i_where.GetExpression()->GetArguments();
                for (size_t i = 0; i < arguments.size(); ++i)
                {
                    const Tensor & argument = arguments[i];

                    IntInterval cardinality = GetCardinality(argument);
                    if (cardinality != IntInterval{ 1, 1 })
                    {
                        DJUP_ASSERT(IsRepetition(argument));

                        ++i_context.m_curr_depth;

                        std::vector<std::shared_ptr<const Expression>> involved_identifiers;
                        GetInvolvedIdentifiers(involved_identifiers, argument);

                        uint32_t repetitions = 0;
                        for (const Substitution & subst : i_context.m_substitutions)
                        {
                            for (const auto & involved_identifier : involved_identifiers)
                            {
                                if (involved_identifier->GetName() == subst.m_identifier_name)
                                {
                                    if (subst.m_value.GetExpression()->GetName() != builtin_names::Tuple)
                                        Error("Expected tuple for variadic substitution of ", subst.m_identifier_name);
                                    uint32_t new_repetitions = NumericCast<uint32_t>(subst.m_value.GetExpression()->GetArguments().size());
                                    if (repetitions != 0 && repetitions != new_repetitions)
                                        Error("Inconsistent repetition count for variadic substitution of ", subst.m_identifier_name);
                                    repetitions = new_repetitions;

                                    for (uint32_t rep = 0; rep < repetitions; ++rep)
                                    {
                                        if (rep < subst.m_value.GetExpression()->GetArguments().size())
                                        {
                                            const auto & subst_val = subst.m_value.GetExpression()->GetArgument(rep).GetExpression();
                                            i_context.m_processed[argument.GetExpression()] = subst_val;
                                        }

                                        new_arguments.push_back(
                                            ApplySubstitutionsImpl(argument, i_context));

                                        some_argument_replaced = some_argument_replaced ||
                                            new_arguments.back().GetExpression() != argument.GetExpression();
                                    }
                                    break;
                                }
                            }
                        }

                        Tensor replacement = ApplySubstitutionsImpl(argument, i_context);

                        DJUP_ASSERT(i_context.m_curr_depth != 0);
                        --i_context.m_curr_depth;
                    }
                    else
                    {
                        new_arguments.push_back(
                            ApplySubstitutionsImpl(argument, i_context));

                        some_argument_replaced = some_argument_replaced ||
                            new_arguments.back().GetExpression() != argument.GetExpression();
                    }
                }
                if (some_argument_replaced)
                {
                    const Expression & expr = *replacement.GetExpression();
                    replacement = MakeExpression(
                        expr.GetType(), expr.GetName(), new_arguments, expr.GetMetadata());
                }

                if (!i_context.m_processed.insert(std::make_pair(
                    i_where.GetExpression(), replacement.GetExpression())).second)
                {
                    Error("SubstituteByPredicate - Cyclic expression");
                }

                return replacement;
            }

        } // unnamed namespace

        Tensor ApplySubstitutions(const Tensor & i_where,
            Span<const Substitution> i_substitutions)
        {
            ApplySubstitutionContext context{ i_substitutions };
            return ApplySubstitutionsImpl(i_where, context);

            /*return SubstituteByPredicate(i_where, [i_substitutions](const Tensor i_tensor) {
                for (const Substitution & subst : i_substitutions)
                {
                    if (i_tensor.GetExpression()->GetName() == subst.m_identifier_name)
                    {
                        return subst.m_value;
                    }
                }
                return i_tensor;
            });*/
        }
        
    } // namespace o2o_pattern

} // namespace djup

