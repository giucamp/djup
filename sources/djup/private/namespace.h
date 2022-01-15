
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <memory>
#include <private/discrimination_net.h>

namespace djup
{
    class Namespace
    {
    public:

        Namespace(Name i_name, const std::shared_ptr<const Namespace> & i_parent);

        // disable copy
        Namespace(const Namespace &) = delete;
        Namespace & operator = (const Namespace &) = delete;

        /* Returns the root namespace, which is always unmodifiable */
        static const std::shared_ptr<const Namespace> & Root();

        std::shared_ptr<const Namespace> const & GetParent() const { return m_parent; }

        void AddScalarType(Name i_name, Span<const Name> i_subsets);

        bool IsScalarType(const Name & i_name) const;

        bool ScalarTypeBelongsTo(const Name & i_target_type, const Name & i_set) const;

        void AddSubstitutionAxiom(const Tensor & i_what, const Tensor & i_with, const Tensor & i_when = {});

        void AddTypeInferenceAxiom(const Tensor & i_what, const Tensor & i_type, const Tensor & i_when = {});

        Tensor Canonicalize(const Tensor & i_source) const;
    
    private:
        
        // scalar types
        struct ScalarType
        {
            Name m_name;
            std::vector<Name> m_subsets;
        };
        std::vector<ScalarType> m_scalar_types;

        // substitution axioms: patterns and righ-hand-side expressions
        DiscriminationNet m_substitution_axioms_patterns;
        std::vector<Tensor> m_substitution_axioms_rhss;

        // type-inference axioms: patterns and righ-hand-side expressions
        DiscriminationNet m_type_inference_axioms_patterns;
        std::vector<Tensor> m_type_inference_axioms_rhss;

        std::shared_ptr<const Namespace> const m_parent;
        Name m_name;

    private:

        void AppendScalarTypeSubsets(const Name & i_name, std::vector<Name> & io_subsets);
        
        const ScalarType * FindScalarType(const Name & i_name) const;

        Tensor ApplySubstitutionAxioms(const Tensor & i_source) const;

        Tensor ApplyTypeInferenceAxioms(const Tensor & i_source) const;
    };

    std::shared_ptr<const Namespace> GetStandardNamespace();

    std::shared_ptr<Namespace> GetDefaultNamespace();

    void SetActiveNamespace(std::shared_ptr<Namespace> i_namespace);

    std::shared_ptr<Namespace> GetActiveNamespace();

    Tensor MakeNamespace(Span<Tensor const> i_arguments);
}
