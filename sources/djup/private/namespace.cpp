
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/namespace.h>
#include <private/substitute_by_predicate.h>
#include <core/algorithms.h>

namespace djup
{
    const std::shared_ptr<const Namespace> & Namespace::Root()
    {
        static const std::shared_ptr<const Namespace> root = std::make_shared<Namespace>("Root", nullptr);
        return root;
    }

    Namespace::Namespace(Name i_name, const std::shared_ptr<const Namespace> & i_parent)
        : m_parent(i_parent), m_name(std::move(i_name))
    {
    }

    void Namespace::AddScalarType(Name i_name, Span<const Name> i_subsets)
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

    void Namespace::AppendScalarTypeSubsets(const Name & i_name, std::vector<Name> & io_subsets)
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

    bool Namespace::IsScalarType(const Name & i_name) const
    {
        return FindScalarType(i_name) != nullptr;
    }

    bool Namespace::ScalarTypeBelongsTo(const Name & i_target_type, const Name & i_set) const
    {
        if(i_target_type == i_set)
            return true;

        if(const ScalarType * type = FindScalarType(i_set))
            return Contains(type->m_subsets, i_target_type);
        
        return false;
    }

    const Namespace::ScalarType * Namespace::FindScalarType(const Name & i_name) const
    {
        const Namespace * curr_namespace = this;
        do {

            for(const ScalarType & scalar_type : curr_namespace->m_scalar_types)
                if(scalar_type.m_name == i_name)
                    return &scalar_type;

            curr_namespace = curr_namespace->m_parent.get();
        } while(curr_namespace != nullptr);

        return nullptr;
    }

    void Namespace::AddSubstitutionAxiom(const Tensor & i_what, const Tensor & i_with, const Tensor & i_when)
    {
        const size_t pattern_id = m_substitution_axioms_rhss.size();
        m_substitution_axioms_rhss.push_back(i_with);
        m_substitution_axioms_patterns.AddPattern(pattern_id, i_what, i_when);
    }

    void Namespace::AddTypeInferenceAxiom(const Tensor & i_what, const Tensor & i_type, const Tensor & i_when)
    {
        const size_t pattern_id = m_type_inference_axioms_rhss.size();
        m_type_inference_axioms_rhss.push_back(i_type);
        m_type_inference_axioms_patterns.AddPattern(pattern_id, i_what, i_when);
    }

    namespace
    {
        struct ApplySubstitutions
        {
            const DiscriminationNet::Match & m_match;

            Tensor operator () (const Tensor & i_candidate) const
            {
                for(const DiscriminationNet::Substitution & substitution : m_match.m_substitutions)
                {
                    if(AlwaysEqual(substitution.m_variable, i_candidate))
                    {
                        return substitution.m_value;
                    }
                }
                return i_candidate;
            }
        };

        struct ApplyType
        {
            const DiscriminationNet::Match & m_match;

            Tensor operator () (const Tensor & i_candidate) const
            {
                for(const DiscriminationNet::Substitution & substitution : m_match.m_substitutions)
                {
                    if(AlwaysEqual(substitution.m_variable, i_candidate))
                    {
                        return substitution.m_value;
                    }
                }
                return i_candidate;
            }
        };
    }

    Tensor Namespace::ApplySubstitutionAxioms(const Tensor & i_source) const
    {
        std::vector<DiscriminationNet::Match> matches;
        m_substitution_axioms_patterns.FindMatches(i_source, matches);
        if(!matches.empty())
        {
            const DiscriminationNet::Match & match = matches[0];
            const Tensor & replacement = m_substitution_axioms_rhss[match.m_pattern_id];
            // if(match.m_substitutions.empty())
            return SubstituteByPredicate(replacement, ApplySubstitutions{match});
        }
        else
            return i_source;
    }

    Tensor Namespace::ApplyTypeInferenceAxioms(const Tensor & i_source) const
    {
        std::vector<DiscriminationNet::Match> matches;
        m_type_inference_axioms_patterns.FindMatches(i_source, matches);
        if(!matches.empty())
        {
            const DiscriminationNet::Match & match = matches[0];
            const Tensor & pattern_type = m_type_inference_axioms_rhss[match.m_pattern_id];
            Tensor type = SubstituteByPredicate(pattern_type, ApplySubstitutions{match});
            const Expression & source = *i_source.GetExpression();
            // to do: check compatibiloity with the previous type
            return MakeExpression(source.GetName(), source.GetArguments(), ExpressionMetadata{type});
        }
        else
            return i_source;
    }

    Tensor Namespace::Canonicalize(const Tensor & i_source) const
    {
        Tensor result = i_source;

        // loop until the expression does not change
        const Expression * prev_expr = result.GetExpression().get();
        do {
            result = ApplyTypeInferenceAxioms(result);

            prev_expr = result.GetExpression().get();
            result = ApplySubstitutionAxioms(result);
        } while(prev_expr != result.GetExpression().get());

        return result;
    }

    namespace
    {
        thread_local std::shared_ptr<Namespace> g_active_namespace = GetDefaultNamespace();

        std::shared_ptr<Namespace> MakeDefaultNamespace()
        {
            std::shared_ptr<Namespace> default_namespace = std::make_shared<Namespace>("Default", GetStandardNamespace());
            return default_namespace;
        }
    }

    std::shared_ptr<Namespace> GetDefaultNamespace()
    {
        static thread_local std::shared_ptr<Namespace> default_namespace = MakeDefaultNamespace();
        return default_namespace;
    }

    void SetActiveNamespace(std::shared_ptr<Namespace> i_namespace)
    {
        g_active_namespace = std::move(i_namespace);
    }

    std::shared_ptr<Namespace> GetActiveNamespace()
    {
        return g_active_namespace;
    }

    NamespaceScope::NamespaceScope(std::shared_ptr<Namespace> i_new_namespace)
        : m_prev_namespace(GetActiveNamespace())
    {
        SetActiveNamespace(std::move(i_new_namespace));
    }

    NamespaceScope::~NamespaceScope()
    {
        if(m_prev_namespace)
            SetActiveNamespace(std::move(m_prev_namespace));
    }

    NamespaceScope::NamespaceScope(NamespaceScope && i_source) noexcept
        : m_prev_namespace(i_source.m_prev_namespace)
    {
        i_source.m_prev_namespace.reset();
    }

    const NamespaceScope & NamespaceScope::operator = (NamespaceScope && i_source) noexcept
    {
        m_prev_namespace = i_source.m_prev_namespace;
        i_source.m_prev_namespace.reset();
        return *this;
    }
}
