
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/scope.h>
#include <private/substitute_by_predicate.h>

namespace djup
{
    const std::shared_ptr<const Scope> & Scope::Root()
    {
        static const std::shared_ptr<const Scope> root = std::make_shared<Scope>(nullptr);
        return root;
    }

    Scope::Scope(const std::shared_ptr<const Scope> & i_parent)
        : m_parent(i_parent)
    {
    }

    void Scope::AddSubstitutionAxiom(const Tensor & i_what, const Tensor & i_with, const Tensor & i_when)
    {
        const size_t pattern_id = m_canonicalization_axioms_replacements.size();
        m_canonicalization_axioms_replacements.push_back(i_with);
        m_canonicalization_axioms_patterns.AddPattern(pattern_id, i_what, i_when);
    }

    namespace
    {
        struct ApplySubstitutions
        {
            const DiscriminationNetwork::Match & m_match;

            Tensor operator () (const Tensor & i_candidate) const
            {
                for(const DiscriminationNetwork::Substitution & substitution : m_match.m_substitutions)
                {
                    if(AlwaysEqual(substitution.m_variable, *i_candidate.GetExpression()))
                    {
                        return MakeExpression(substitution.m_value.GetName(), 
                            substitution.m_value.GetType(), substitution.m_value.GetArguments());
                    }
                }
                return i_candidate;
            }
        };
    }

    Tensor Scope::Canonicalize(const Tensor & i_source) const
    {
        std::vector<DiscriminationNetwork::Match> matches;
        m_canonicalization_axioms_patterns.FindMatches(i_source, matches);
        if(!matches.empty())
        {
            const DiscriminationNetwork::Match & match = matches[0];
            const Tensor & replacement = m_canonicalization_axioms_replacements[match.m_pattern_id];

            return SubstituteByPredicate(i_source, ApplySubstitutions{match});
        }
        else
            return i_source;
    }
}
