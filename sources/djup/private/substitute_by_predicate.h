
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <private/common.h>
#include <private/make_expr.h>
#include "djup/tensor.h"
#include <djup/expression.h>
#include <unordered_map>

namespace djup
{
    namespace detail
    {
        using ReplacementMap = std::unordered_map<
            std::shared_ptr<const Expression>,
            std::shared_ptr<const Expression> >;

        template <typename PREDICATE>
            Tensor SubstituteByPredicateImpl(const Tensor & i_where, 
                const PREDICATE & i_predicate,
                ReplacementMap & i_replacement_map)
        {
            auto const it = i_replacement_map.find(i_where.GetExpression());
            if(it != i_replacement_map.end())
                return Tensor(it->second);

            Tensor replacement = i_predicate(i_where);

            std::vector<Tensor> new_arguments;
            new_arguments.reserve(replacement.GetExpression()->GetArguments().size());
        
            bool some_operand_replaced = false;
            for(const Tensor & operand : replacement.GetExpression()->GetArguments())
            {
                new_arguments.push_back(
                    SubstituteByPredicateImpl(operand, i_predicate, i_replacement_map));

                some_operand_replaced = some_operand_replaced || 
                    new_arguments.back().GetExpression() != operand.GetExpression();
            }
            if(some_operand_replaced)
            {
                const Expression & expr = *replacement.GetExpression();
                replacement = MakeExpression(expr.GetName(), new_arguments, expr.GetMetadata());
            }

            if(!i_replacement_map.insert(std::make_pair(
                i_where.GetExpression(), replacement.GetExpression())).second)
            {
                Error("SubstituteByPredicate - Cyclic expression");
            }

            return replacement;
        }

    } // namespace detail

    /** Tries to apply a substitution to an expression and all subexpressions.
        The predicate must take a single tensor argument and must return tensor.
        For every subexpression the predicate is invoked. The predicate can return the
        its argument to signal that the substituion was not done, or it can return a 
        tensor bound to a different expression (in this case a substitution was performed). */
    template <typename PREDICATE>
        [[nodiscard]] Tensor SubstituteByPredicate(
            const Tensor & i_where,
            const PREDICATE & i_predicate)
    {
        detail::ReplacementMap replacement_map;
        return detail::SubstituteByPredicateImpl(i_where, i_predicate, replacement_map);
    }

    /** Tries to apply a substitution to a whole expression graph.
        The predicate must take a single tensor argument and must return tensor.
        For every subexpression the predicate is invoked. The predicate can return the
        its argument to signal that the substituion was not done, or it can return a 
        tensor bound to a different expression (in this case a substitution was performed). */
    template <typename PREDICATE>
        [[nodiscard]] std::vector<Tensor> SubstituteByPredicate(
            Span<const Tensor> i_where,
            const PREDICATE & i_predicate)
    {
        detail::ReplacementMap replacement_map;

        std::vector<Tensor> result;
        result.reserve(i_where.size());
        for(auto const & where : i_where)
            result.push_back(detail::SubstituteByPredicateImpl(where, i_predicate, replacement_map));

        return result;
    }
}
