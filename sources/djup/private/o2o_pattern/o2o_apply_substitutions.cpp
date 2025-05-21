
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
                const Namespace & m_namespace;

                Span<const Substitution> m_substitutions;

                std::unordered_map<std::shared_ptr<const Expression>,
                    std::shared_ptr<const Expression> > m_processed;

                uint32_t m_curr_depth{ 0 };
            };

            Tensor SubstituteSingleIdentifier(const Namespace & i_namespace, 
                const Tensor & i_where, const Name & i_name, const Tensor & i_with)
            {
                return SubstituteByPredicate(i_namespace, i_where, [&i_name, &i_with](const Tensor i_tensor) {
                    if (i_tensor.GetExpression()->GetName() == i_name)
                        return i_with;
                    else
                        return i_tensor;
                });
            }

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

                    UIntInterval cardinality = GetCardinality(argument);
                    if (cardinality != UIntInterval{ 1, 1 })
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
                                            const Tensor subst_val = subst.m_value.GetExpression()->GetArgument(rep).GetExpression();

                                            // apply the substitution to the first (and only) argument of the repetition
                                            Tensor rep_arg = argument.GetExpression()->GetArgument(0);
                                            const auto subst_expr = SubstituteSingleIdentifier(
                                                i_context.m_namespace, rep_arg.GetExpression(), 
                                                subst.m_identifier_name, subst_val);

                                            i_context.m_processed[argument.GetExpression()] = subst_expr.GetExpression();
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
                    replacement = MakeExpression(i_context.m_namespace,
                        expr.GetType(), expr.GetName(), new_arguments, expr.GetMetadata());
                }

                if (!i_context.m_processed.insert(std::make_pair(
                    i_where.GetExpression(), replacement.GetExpression())).second)
                {
                    Error("SubstituteByPredicate - Cyclic expression");
                }

                return replacement;
            }

        } // anonymous namespace

        Tensor ApplySubstitutions(const Namespace & i_namespace, 
            const Tensor & i_where, Span<const Substitution> i_substitutions)
        {
            ApplySubstitutionContext context{ i_namespace, i_substitutions, {}, {} };
            return ApplySubstitutionsImpl(i_where, context);
        }
        
    } // namespace o2o_pattern

} // namespace djup

