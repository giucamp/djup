
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <private/substitute_by_predicate.h>
#include <private/o2o_pattern/o2o_pattern_match.h>
#include <private/builtin_names.h>
#include <private/o2o_pattern/o2o_pattern_info.h>
#include <core/flags.h>
#include <core/pool.h>

namespace djup
{
    namespace o2o_pattern
    {
        namespace
        {
            Tensor PreprocessPattern(const Tensor & i_pattern)
            {
                return SubstituteByPredicate(i_pattern, [](const Tensor & i_candidate) {
                    FunctionFlags flags = GetFunctionFlags(*i_candidate.GetExpression());

                    bool some_substitution = false;
                    std::vector<Tensor> new_arguments;

                    const std::vector<Tensor> & arguments = i_candidate.GetExpression()->GetArguments();
                    const size_t argument_count = arguments.size();

                    // substitute identifiers in associative functions with AssociativeIdentifier()
                    if (HasFlag(flags, FunctionFlags::Associative))
                    {
                        size_t index = 0;

                        for (; index < argument_count; index++)
                        {
                            const Tensor & argument = arguments[index];
                            if (!IsConstant(argument))
                            {
                                new_arguments = arguments;
                                some_substitution = true;
                                break;
                            }
                        }

                        for (; index < argument_count; index++)
                        {
                            const Tensor & argument = arguments[index];
                            if (!IsConstant(argument))
                            {
                                new_arguments[index] = MakeExpression(
                                    argument.GetExpression()->GetType(),
                                    builtin_names::AssociativeIdentifier,
                                    { argument },
                                    argument.GetExpression()->GetMetadata());
                            }
                        }
                    }

                    if (some_substitution)
                        return MakeExpression(
                            i_candidate.GetExpression()->GetType(),
                            i_candidate.GetExpression()->GetName(),
                            new_arguments, i_candidate.GetExpression()->GetMetadata());
                    else
                        return i_candidate;
                    });
            }
        }

        struct PatternSegment
        {
            FunctionFlags m_flags = FunctionFlags::None;
            Span<const Tensor> m_pattern;
            Span<const ArgumentInfo> m_arg_infos;

            PatternSegment() = default;

            PatternSegment(FunctionFlags i_flags, Span<const Tensor> i_pattern, Span<const ArgumentInfo> i_arguments)
                : m_flags(i_flags), m_pattern(i_pattern), m_arg_infos(i_arguments)
            {
                DJUP_ASSERT(m_arg_infos.size() == m_pattern.size());
            }
        };


        struct GraphNode
        {
            size_t m_outgoing_edges{};
        };

        struct Candidate
        {
            uint32_t m_start_node{};
            uint32_t m_dest_node{};
            Span<const Tensor> m_target_arguments;
            PatternSegment m_pattern;
            uint32_t m_repetitions = 0;
            uint32_t m_version{};
            bool m_decayed = false;
            uint32_t m_open{};
            uint32_t m_close{};
            std::vector<Substitution> m_substitutions;
        };

        struct Edge
        {
            uint32_t m_source_index{};
            Pool<Candidate>::Handle m_candidate_ref;
            std::vector<Substitution> m_substitutions;
            uint32_t m_open;
            uint32_t m_close;
        };

        struct MatchingContext
        {
            Pool<Candidate> m_candidates;
            std::vector<GraphNode> m_graph_nodes;
            std::unordered_multimap<uint32_t, Edge> m_edges; // the key is the destination node
            std::unordered_map<const Expression*, PatternInfo> m_pattern_infos;
        };

        Pattern::Pattern(const Tensor & i_pattern, const Tensor & i_when)
        {
            m_pattern = PreprocessPattern(i_pattern);
        }

        MatchResult Pattern::Match(const Tensor & i_target) const
        {
            return {};
        }

    } // namespace o2o_pattern

} // namespace djup

