
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/scope.h>
#include <private/substitute_by_predicate.h>
#include <core/algorithms.h>

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

    void Scope::AddScalarType(Name i_name, Span<const Name> i_subsets)
    {
        if(IsScalarType(i_name))
            Error("Scalar type ", i_name, " already defined");

        ScalarType type;
        type.m_name = std::move(i_name);
        for(const Name & subset : i_subsets)
        {
            if(!Contains(type.m_subsets, subset))
                type.m_subsets.push_back(subset);
            AppendScalarTypeSubsets(subset, type.m_subsets);
        }
        m_scalar_types.push_back(std::move(type));
    }

    void Scope::AppendScalarTypeSubsets(const Name & i_name, std::vector<Name> io_subsets)
    {
        if(const ScalarType * type = FindScalarType(i_name))
        {
            for(const Name & subset : type->m_subsets)
            {
                if(!Contains(io_subsets, subset))
                    io_subsets.push_back(subset);
            }
        }
    }

    bool Scope::IsScalarType(const Name & i_name) const
    {
        return FindScalarType(i_name) != nullptr;
    }

    bool Scope::ScalarTypeBelongsTo(const Name & i_target_type, const Name & i_set) const
    {
        if(i_target_type == i_set)
            return true;

        if(const ScalarType * type = FindScalarType(i_set))
            return Contains(type->m_subsets, i_target_type);
        
        return false;
    }

    const Scope::ScalarType * Scope::FindScalarType(const Name & i_name) const
    {
        const Scope * scope = this;
        do {

            for(const ScalarType & scalar_type : scope->m_scalar_types)
                if(scalar_type.m_name == i_name)
                    return &scalar_type;

            scope = scope->m_parent.get();
        } while(scope != nullptr);

        return nullptr;
    }

    void Scope::AddSubstitutionAxiom(const Tensor & i_what, const Tensor & i_with, const Tensor & i_when)
    {
        const size_t pattern_id = m_canonicalization_axioms_replacements.size();
        m_canonicalization_axioms_replacements.push_back(i_with);
        m_canonicalization_axioms_patterns.AddPattern(pattern_id, i_what, i_when);
    }

    void Scope::AddSubstitutionAxiom(std::string_view i_what, std::string_view i_with, std::string_view i_when)
    {
        AddSubstitutionAxiom(Tensor(i_what), Tensor(i_with), Tensor(i_when));
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
                            substitution.m_value.GetArguments());
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
            // if(match.m_substitutions.empty())
            return SubstituteByPredicate(replacement, ApplySubstitutions{match});
        }
        else
            return i_source;
    }

    namespace
    {
        thread_local std::shared_ptr<Scope> g_active_scope = GetDefaultScope();

        std::shared_ptr<Scope> MakeDefaultScope()
        {
            std::shared_ptr<Scope> scope = std::make_shared<Scope>(GetStandardScope());
            return scope;
        }
    }

    std::shared_ptr<Scope> GetDefaultScope()
    {
        static thread_local std::shared_ptr<Scope> default_scope = MakeDefaultScope();
        return default_scope;
    }

    void SetActiveScope(std::shared_ptr<Scope> i_scope)
    {
        g_active_scope = std::move(i_scope);
    }

    std::shared_ptr<Scope> GetActiveScope()
    {
        return g_active_scope;
    }
}
