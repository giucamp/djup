
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <private/common.h>
#include <private/expression.h>
#include <vector>
#include <optional>

namespace djup
{
    class Namespace;

    namespace o2o_pattern
    {
        struct Substitution
        {
            Name m_identifier_name;
            Tensor m_value;
        };

        struct MatchResult
        {
            std::vector<Substitution> m_substitutions;
        };

        class Pattern
        {
        public:

            Pattern(const Namespace & i_namespace, const Tensor & i_pattern);

            Pattern(const Namespace & i_namespace, 
                const Tensor & i_pattern, const Tensor & i_when);

            std::optional<MatchResult> MatchOne(const Tensor & i_target, 
                const char * i_artifact_path) const;

            std::vector<MatchResult> MatchAll(const Tensor & i_target,
                const char * i_artifact_path) const;

        private:
            Tensor m_pattern;
            Tensor m_when;
            const Namespace & m_namespace;
        };

        Tensor ApplySubstitutions(const Tensor & i_where,
            Span<const Substitution> i_substitutions);

    } // namespace o2o_pattern

} // namespace djup
