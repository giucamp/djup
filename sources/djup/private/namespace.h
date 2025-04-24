
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <private/common.h>
#include <memory>
#include <private/pattern/discrimination_tree.h>
#include <core/name.h>

namespace djup
{
    /** A namespace is a named object which contains declarations (axioms and types).
        Every namespace has a parent namespace, from which inherits all declarations 
        with recursion. There is a global root immutable namespace, which does not 
        have a parent, from which all namespaces directly or indirectly inherit. */
    class Namespace
    {
    public:

        /** If i_parent is null, the parent will be set to the root namespace. */
        Namespace(Name i_name, const std::shared_ptr<const Namespace> & i_parent);

        // disable copy
        Namespace(const Namespace &) = delete;
        Namespace & operator = (const Namespace &) = delete;

        /* Returns the root namespace, which is always unmodifiable */
        static const std::shared_ptr<const Namespace> & Root();

        std::shared_ptr<const Namespace> const & GetParent() const { return m_parent; }

        void AddScalarType(Name i_name, Span<const Name> i_subsets);

        bool IsScalarType(const Name & i_name) const;

        bool ScalarTypeBelongsTo(const Name& i_target_type, const Name& i_set) const;

        bool TypeBelongsTo(const TensorType & i_element, const TensorType & i_set) const;

        void AddSubstitutionAxiom(const Tensor & i_what, const Tensor & i_with, const Tensor & i_when = {});

        void AddTypeInferenceAxiom(const Tensor & i_what, const Tensor & i_type, const Tensor & i_when = {});

        void SetDescribingExpression(const Tensor & i_expr) { m_describing_expression = i_expr; }

        const Tensor & GetDescribingExpression() const { return m_describing_expression; }

        /* Applies type inference and substitutions axioms to an expr until 
           the result doesn't chance anymore */
        Tensor Canonicalize(const Tensor & i_source) const;

        std::string SubstitutionGraphToDotLanguage() const;
    
    private:
        
        // scalar types
        struct ScalarType
        {
            Name m_name;
            std::vector<Name> m_subsets;
        };
        std::vector<ScalarType> m_scalar_types;

        // substitution axioms: patterns and right-hand-side expressions
        pattern::DiscriminationTree m_substitution_axioms_patterns;
        std::vector<Tensor> m_substitution_axioms_rhss;

        // type-inference axioms: patterns and right-hand-side expressions
        pattern::DiscriminationTree m_type_inference_axioms_patterns;
        std::vector<Tensor> m_type_inference_axioms_rhss;

        /* identifiers */
        std::unordered_map<Name, Tensor> m_identifiers;

        // namespace data
        std::shared_ptr<const Namespace> m_parent;
        Name m_name;
        Tensor m_describing_expression;

    private:

        struct TagRoot {};

        Namespace(Namespace::TagRoot);

        void AppendScalarTypeSubsets(const Name & i_name, std::vector<Name> & io_subsets);
        
        const ScalarType * FindScalarType(const Name & i_name) const;

        Tensor ApplySubstitutionAxioms(const Tensor & i_source) const;

        // Tensor ApplyTypeInferenceAxioms(const Tensor & i_source) const;
    };

    std::shared_ptr<const Namespace> GetStandardNamespace();
}
