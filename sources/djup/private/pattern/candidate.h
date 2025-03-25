
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <private/expression.h>
#include <private/pattern/pattern_info.h>
#include <private/pattern/discrimination_net.h>

namespace djup
{
    namespace pattern
    {
        struct Substitution
        {
            Name m_variable_name;
            Tensor m_value;
        };

        struct CandidateData
        {
            uint16_t m_pattern_offset{};
            uint16_t m_target_offset{};

            Span<const Tensor> m_targets;

            uint32_t m_discrimination_node{ std::numeric_limits<uint32_t>::max() };
            const DiscriminationNet::Edge* m_discrimination_edge{ nullptr };

            uint32_t m_repetitions_offset{};
            uint32_t m_repetitions{ std::numeric_limits<uint32_t>::max() };

            uint32_t m_open{};
            uint32_t m_close{};

            std::vector<Substitution> m_substitutions;

            bool AddSubstitution(const Name& i_variable_name, const Tensor& i_value)
            {
                m_substitutions.emplace_back(Substitution{ i_variable_name, i_value });
                return true;
            }
        };

        struct Candidate
        {
            uint32_t m_start_node{};
            uint32_t m_end_node{};

            CandidateData m_data;

            uint32_t m_version{};
            bool m_decayed = false;
        };

        std::vector<Span<const Tensor>> Tokenize(const Tensor& i_tensor);

        std::string TensorSpanToString(Span<const Tensor> i_tensor);
    
    } // namespace pattern

} // namespace djup
