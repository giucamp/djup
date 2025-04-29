
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <private/common.h>
#include <djup/tensor.h>
#include <core/name.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace djup
{
    namespace pattern
    {
        struct Substitution
        {
            Name m_identifier_name;
            Tensor m_value;
        };

        class SubstitutionsBuilder
        {
        public:

            bool Add(
                const std::vector<Substitution>& i_substitutions,
                uint32_t i_open, uint32_t i_close);

            const std::vector<Substitution>& GetSubstitutions() const
            {
                return m_substitutions;
            }

        private:

            struct VariadicValue
            {
                std::vector<std::vector<Tensor>> m_stack;
            };

        private:

            bool AddToBottomLayer(const Substitution & i_solution);

            bool AddToBottomLayer(
                const std::vector<Substitution>& i_source_substitutions);

            void AddToVariadic(
                const std::vector<Substitution>& i_source_substitutions);

            static Tensor VariadicClear(VariadicValue& i_dest);

            static void VariadicReduceDepth(VariadicValue& i_dest);

            static Tensor ToTuple(const std::vector<Tensor>& i_source);

        private:
            uint32_t m_curr_depth{};
            std::vector<Substitution> m_substitutions;
            std::unordered_map<Name, VariadicValue> m_variadic_substitutions;
        };
    
    } // namespace pattern

} // namespace djup
