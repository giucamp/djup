
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <private/common.h>
#include <djup/expression.h>
#include <vector>

namespace djup
{
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

            Pattern(const Tensor & i_pattern, const Tensor & i_when);

            MatchResult Match(const Tensor & i_target) const;

        private:
            Tensor m_pattern;
        };

    } // namespace o2o_pattern

} // namespace djup
