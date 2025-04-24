
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <private/common.h>
#include <djup/tensor.h>
#include <core/name.h>
#include <djup/expression.h>
#include <variant>

namespace djup
{
    struct PatternMatch
    {
        struct VariableValue
        {
            std::variant<std::monostate, Tensor, std::vector<VariableValue>> m_value;
        };

        size_t m_pattern_id{};
        std::unordered_map<Name, Tensor> m_substitutions;
    };

    size_t PatternMatchingCount(const Tensor & i_target, const Tensor & i_pattern);

    PatternMatch Match(const Tensor & i_target, const Tensor & i_pattern);

    Tensor SubstitutePatternMatch(const Tensor & i_source, const PatternMatch & i_match);
}
