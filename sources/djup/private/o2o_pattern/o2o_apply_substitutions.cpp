
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

                std::vector<Name> m_identifiers_to_remove;

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

            void GetInvolvedIdentifiers(std::vector<Tensor> & o_involved, const Tensor & i_expr)
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

            uint32_t GetInvolvedCardinality(
                ApplySubstitutionContext & i_context,
                const std::vector<Tensor> & i_involved_identifiers)
            {
                const auto infinite = std::numeric_limits<uint32_t>::max();
                uint32_t cardinality = infinite;

                for (const auto & involved : i_involved_identifiers)
                {
                    const Expression * subst_val = nullptr;
                    for (const Substitution & subst : i_context.m_substitutions)
                    {
                        if (subst.m_identifier_name == involved.GetExpression()->GetName())
                        {
                            subst_val = subst.m_value.GetExpression().get();
                            break;
                        }
                    }
                    if (subst_val == nullptr)
                    {
                        if(cardinality == infinite)
                            cardinality = 0;
                        else
                            Error("Mismatching cardinality for variadic substitution of ",
                                involved.GetExpression()->GetName(), " (", cardinality, " and 0)");
                        continue;
                    }

                    if (subst_val->GetName() == builtin_names::Tuple)
                    {
                        const uint32_t this_cardinality = static_cast<uint32_t>(
                            subst_val->GetArguments().size());

                        if (cardinality == infinite)
                        {
                            cardinality = this_cardinality;
                        }
                        else if(cardinality != this_cardinality)
                        {
                            Error("Mismatching cardinality for variadic substitution of ",
                                involved.GetExpression()->GetName(), " (", cardinality, 
                                " and ", this_cardinality, ")");
                        }
                    }
                    else
                    {
                        Error("Non-tuple expression in variadic substitution: ",
                            involved.GetExpression()->GetName());
                    }
                }
                return cardinality;
            }
        
            Tensor ApplySubstitutionsImpl(const Tensor & i_where,
                ApplySubstitutionContext & i_context)
            {
                auto const it = i_context.m_processed.find(i_where.GetExpression());
                if (it != i_context.m_processed.end())
                    return Tensor(it->second);

                //PrintLn();
                //PrintLn("Substitute in ", ToSimplifiedString(i_where));

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
#
                        some_argument_replaced = true;

                        // enum all identifiers appearing in the repetition (...)
                        std::vector<Tensor> involved_identifiers;
                        GetInvolvedIdentifiers(involved_identifiers, argument);
                        
                        /* get a common cardinality of the tuple arguments 
                           (or raise an error) */
                        const uint32_t cardinality = GetInvolvedCardinality(i_context, involved_identifiers);
                        
                        // for each repetition
                        for (uint32_t repetition = 0; repetition < cardinality; repetition++)
                        {
                            /* construct a context with every i-th tuple argument */
                            std::vector<Substitution> substitutions(
                                i_context.m_substitutions.begin(), i_context.m_substitutions.end());
                            for (Substitution & subst : substitutions)
                            {
                                if (!AnyOf(involved_identifiers, 
                                    [&subst](const Tensor & i_tensor) {
                                        return i_tensor.GetExpression()->GetName() ==
                                            subst.m_identifier_name; }))
                                    continue;

                                DJUP_ASSERT(subst.m_value.GetExpression()->GetName() == builtin_names::Tuple);
                                subst.m_value = subst.m_value.GetExpression()->GetArgument(repetition);
                            }
                            ApplySubstitutionContext context = i_context;
                            context.m_substitutions = substitutions;

                            const Tensor rep_argument = argument.GetExpression()->GetArgument(0);

                            new_arguments.push_back(
                                ApplySubstitutionsImpl(rep_argument, context));
                        }

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

