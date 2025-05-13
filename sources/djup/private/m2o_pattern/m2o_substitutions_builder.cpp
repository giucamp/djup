
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <private/m2o_pattern/m2o_substitutions_builder.h>

namespace djup
{
    namespace m2o_pattern
    {
        bool SubstitutionsBuilder::AddToBottomLayer(const Substitution & i_substitution)
        {
            for(const Substitution & exiting_substitution : m_substitutions)
            {
                if (exiting_substitution.m_identifier_name == i_substitution.m_identifier_name)
                {
                    if (!AlwaysEqual(exiting_substitution.m_value, i_substitution.m_value))
                    {
                        return false;
                    }
                }
            }
            m_substitutions.push_back(i_substitution);
            return true;
        }

        /* Add the source substitutions to the dest vector. If there is
            a contradiction returns false, otherwise true. */
        bool SubstitutionsBuilder::AddToBottomLayer(
            const std::vector<Substitution> & i_source_substitutions)
        {
            for (const Substitution & substitution : i_source_substitutions)
            {
                if (!AddToBottomLayer(substitution))
                    return false;
            }
            // no contradictions
            return true;
        }

        void SubstitutionsBuilder::AddToVariadic(
            const std::vector<Substitution>& i_source_substitutions)
        {
            for (const Substitution& subst : i_source_substitutions)
            {
                VariadicValue& variadic_value = m_variadic_substitutions[subst.m_identifier_name];
                if (variadic_value.m_stack.size() < m_curr_depth)
                    variadic_value.m_stack.resize(m_curr_depth);
                variadic_value.m_stack.back().push_back(subst.m_value);
            }
        }

        Tensor SubstitutionsBuilder::VariadicClear(VariadicValue& i_dest)
        {
            DJUP_ASSERT(i_dest.m_stack.size() >= 1);

            while (i_dest.m_stack.size() > 1)
                VariadicReduceDepth(i_dest);

            Tensor result = ToTuple(i_dest.m_stack.front());
            i_dest.m_stack.clear();
            return result;
        }

        void SubstitutionsBuilder::VariadicReduceDepth(VariadicValue& i_dest)
        {
            DJUP_ASSERT(i_dest.m_stack.size() >= 2);

            const size_t size = i_dest.m_stack.size();
            i_dest.m_stack[size - 2].push_back(Tuple(i_dest.m_stack[size - 1]));
            i_dest.m_stack.pop_back();
        }

        Tensor SubstitutionsBuilder::ToTuple(const std::vector<Tensor>& i_source)
        {
            std::vector<Tensor> arguments;
            arguments.reserve(i_source.size());
            for (auto it = i_source.begin(); it != i_source.end(); ++it)
                arguments.push_back(*it);
            return Tuple(arguments);
        }

        bool SubstitutionsBuilder::Add(
            const std::vector<Substitution>& i_substitutions)
        {
            if (m_curr_depth == 0)
            {
                return AddToBottomLayer(i_substitutions);
            }
            else
            {                
                AddToVariadic(i_substitutions);

     
                return true;
            }
        }

        void SubstitutionsBuilder::Open(uint32_t i_depth)
        {
            m_curr_depth += i_depth;
        }

        bool SubstitutionsBuilder::Close(uint32_t i_depth)
        {
            DJUP_ASSERT(m_curr_depth >= i_depth); // detects underflow
            m_curr_depth -= i_depth;

            if (m_curr_depth == 0)
            {
                for (auto& var_subst : m_variadic_substitutions)
                {
                    Tensor value = VariadicClear(var_subst.second);
                    if (!AddToBottomLayer({ var_subst.first, value }))
                    {
                        return false;
                    }
                }
                m_variadic_substitutions.clear();
            }
            else
            {
                for (auto& var_subst : m_variadic_substitutions)
                {
                    while (var_subst.second.m_stack.size() > m_curr_depth)
                        VariadicReduceDepth(var_subst.second);
                }
            }

            return true;
        }

        const std::vector<Substitution> & SubstitutionsBuilder::GetSubstitutions() const
        {
            DJUP_ASSERT(m_curr_depth == 0);
            return m_substitutions;
        }

    } // namespace m2o_pattern

} // namespace djup
