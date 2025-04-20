
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <private/namespace.h>
#include <private/pattern/substitution_graph.h>
#include <private/expression.h>
#include <private/builtin_names.h>
#include <core/algorithms.h>

namespace djup
{
    const std::shared_ptr<const Namespace> & Namespace::Root()
    {
        static const std::shared_ptr<const Namespace> root = std::shared_ptr<Namespace>(
            new Namespace( TagRoot{} ) );
        return root;
    }

    Namespace::Namespace(Namespace::TagRoot)
        : m_parent(nullptr), m_name("Root")
    {
    }

    Namespace::Namespace(Name i_name, const std::shared_ptr<const Namespace> & i_parent)
        : m_parent(i_parent), m_name(std::move(i_name))
    {
        if (m_parent == nullptr)
            m_parent = Root();
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
        m_substitution_axioms_patterns.AddPattern(NumericCast<int32_t>(pattern_id), i_what, i_when);

        const Expression * what_expr = i_what.GetExpression().get();
        if (what_expr->GetArguments().empty())
        {
            auto metadata = what_expr->GetMetadata();
            if (metadata && metadata->m_type.GetExpression() != nullptr)
            {
                // it's an identifier
                auto res = m_identifiers.insert(std::make_pair(what_expr->GetName(), i_with));
                if (!res.second)
                    Error("Identifier ", what_expr->GetName(), " already declared");
            }
            else
            {
                auto it = m_identifiers.find(what_expr->GetName());
                if (it == m_identifiers.end())
                    Error("Undeclared identifier ", what_expr->GetName());
            }
        }
    }

    void Namespace::AddTypeInferenceAxiom(const Tensor & i_what, const Tensor & i_type, const Tensor & i_when)
    {
        const size_t pattern_id = m_type_inference_axioms_rhss.size();
        m_type_inference_axioms_rhss.push_back(i_type);
        m_type_inference_axioms_patterns.AddPattern(NumericCast<int32_t>(pattern_id), i_what, i_when);
    }

    Tensor Namespace::ApplySubstitutionAxioms(const Tensor & i_source) const
    {
        std::vector<PatternMatch> matches;

        pattern::SubstitutionGraph substitution_graph(m_substitution_axioms_patterns);
        
        substitution_graph.FindMatches(i_source);

        if(!matches.empty())
        {
            const PatternMatch & match = matches[0];
            const Tensor & replacement = m_substitution_axioms_rhss[match.m_pattern_id];
            return SubstitutePatternMatch(replacement, match);
        }
        else
            return i_source;
    }

    Tensor Namespace::ApplyTypeInferenceAxioms(const Tensor & i_source) const
    {
        std::vector<PatternMatch> matches;

        pattern::SubstitutionGraph substitution_graph(m_type_inference_axioms_patterns);

        substitution_graph.FindMatches(i_source);

        if(!matches.empty())
        {
            const PatternMatch & match = matches[0];
            const Tensor & pattern_type = m_type_inference_axioms_rhss[match.m_pattern_id];
            Tensor type = SubstitutePatternMatch(pattern_type, match);
            const Expression & source = *i_source.GetExpression();
            // to do: check compatibility with the previous type
            return MakeExpression(source.GetName(), source.GetArguments(), ExpressionMetadata{type});
        }
        else
            return i_source;
    }

    Tensor Namespace::Canonicalize(const Tensor & i_source) const
    {
        Tensor result(i_source);

        // loop until the expression does not change
        const Expression * prev_expr = result.GetExpression().get();
        do {
            result = ApplyTypeInferenceAxioms(result);

            prev_expr = result.GetExpression().get();
            result = ApplySubstitutionAxioms(result);
        } while(prev_expr != result.GetExpression().get());

        return result;
    }

    std::string Namespace::SubstitutionGraphToDotLanguage() const
    {
        return m_substitution_axioms_patterns.ToGraphWiz("Substitutions").ToDotLanguage();
    }

    namespace
    {
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
}
