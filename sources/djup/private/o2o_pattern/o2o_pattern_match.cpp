
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <private/namespace.h>
#include <private/substitute_by_predicate.h>
#include <private/o2o_pattern/o2o_pattern_match.h>
#include <private/o2o_pattern/o2o_pattern_info.h>
#include <private/o2o_pattern/o2o_debug_utils.h>
#include <private/builtin_names.h>
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

        StringBuilder & operator << (StringBuilder & i_dest, const Candidate & i_source)
        {
            i_dest << "Pattern: " << TensorSpanToString(i_source.m_pattern.m_pattern);
            if (i_source.m_repetitions != 0)
                i_dest << " (" << i_source.m_repetitions << " times)";
            i_dest.NewLine();
            i_dest << " Target: " << TensorSpanToString(i_source.m_target_arguments);
            return i_dest;
        }

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
            const Namespace * m_namespace;
            Pool<Candidate> m_candidates;
            std::vector<GraphNode> m_graph_nodes;
            std::unordered_multimap<uint32_t, Edge> m_edges; // the key is the destination node
            std::unordered_map<const Expression*, PatternInfo> m_pattern_infos;
        };

        const PatternInfo & GetPatternInfo(MatchingContext & i_context, const Tensor & i_pattern)
        {
            const Expression * expr = i_pattern.GetExpression().get();
            auto it = i_context.m_pattern_infos.find(expr);
            if (it != i_context.m_pattern_infos.end())
                return it->second;
            auto res = i_context.m_pattern_infos.insert({ expr, BuildPatternInfo(i_pattern) });
            DJUP_ASSERT(res.second);
            return res.first->second;
        }

        void AddCandidate(MatchingContext & i_context,
            uint32_t i_start_node, uint32_t i_dest_node,
            Span<const Tensor> i_target, PatternSegment i_pattern,
            uint32_t i_open, uint32_t i_close,
            uint32_t i_repetitions = std::numeric_limits<uint32_t>::max())
        {
            DJUP_ASSERT(i_start_node != i_dest_node);

            auto cand_handle = i_context.m_candidates.New();
            Candidate new_candidate = i_context.m_candidates.GetObject(cand_handle);
            new_candidate.m_start_node = i_start_node;
            new_candidate.m_dest_node = i_dest_node;
            new_candidate.m_pattern = i_pattern;
            new_candidate.m_target_arguments = i_target;
            new_candidate.m_repetitions = i_repetitions;
            new_candidate.m_open = i_open;
            new_candidate.m_close = i_close;

            i_context.m_edges.insert({ i_dest_node, Edge{i_start_node, cand_handle, {}, i_open, i_close } });
            i_context.m_graph_nodes[i_start_node].m_outgoing_edges++;
        }

        class LinearPath
        {
        public:

            LinearPath(MatchingContext & i_context, const Candidate & i_source_candidate)
                : m_context(i_context),
                m_start_node(i_source_candidate.m_start_node), m_dest_node(i_source_candidate.m_dest_node),
                m_open(i_source_candidate.m_open), m_close(i_source_candidate.m_close)
            {

            }

            LinearPath(const LinearPath &) = delete;
            LinearPath & operator = (const LinearPath &) = delete;

            void AddEdge(Span<const Tensor> i_target, PatternSegment i_pattern,
                bool i_increase_depth = false, uint32_t i_repetitions = std::numeric_limits<uint32_t>::max())
            {
                /*std::string rep_str;
                if(i_repetitions != std::numeric_limits<uint32_t>::max())
                    rep_str = ToString(" x", i_repetitions);
                PrintLn("Pattern: ", TensorListToString(i_pattern.m_pattern), rep_str);
                PrintLn("Target: ", TensorListToString(i_target));*/

                if (!i_target.empty() && !i_pattern.m_pattern.empty() && i_repetitions != 0)
                {
                    if (!(m_target.empty() && m_pattern.m_pattern.empty()) && m_repetitions != 0)
                    {
                        const uint32_t intermediate_node = NumericCast<uint32_t>(m_context.m_graph_nodes.size());
                        m_context.m_graph_nodes.emplace_back();

                        FlushPendingEdgeIfNotEmpty(intermediate_node);

                        m_start_node = intermediate_node;
                    }

                    // store the pending edge
                    m_target = i_target;
                    m_pattern = i_pattern;
                    m_repetitions = i_repetitions;
                    m_increase_depth = i_increase_depth;
                }
            }

            ~LinearPath() noexcept(false)
            {
                if (m_close == 0)
                    FlushPendingEdgeIfNotEmpty(m_dest_node);
                else
                    FlushPendingEdge(m_dest_node, m_close);
            }

        private:

            void FlushPendingEdgeIfNotEmpty(uint32_t i_dest_node)
            {
                if (!(m_target.empty() && m_pattern.m_pattern.empty()) && m_repetitions != 0)
                {
                    FlushPendingEdge(i_dest_node);
                }
            }


            void FlushPendingEdge(uint32_t i_dest_node, uint32_t i_close = {})
            {
                uint32_t open = m_open;
                if (m_increase_depth)
                {
                    open++;
                    i_close++;
                }
                AddCandidate(m_context, m_start_node, i_dest_node, m_target, m_pattern, open, i_close, m_repetitions);
                m_open = 0;
            }

        private:
            MatchingContext & m_context;
            uint32_t m_start_node;
            uint32_t m_dest_node;
            uint32_t m_open;
            uint32_t m_close;

            // pending edge
            Span<const Tensor> m_target;
            PatternSegment m_pattern;
            uint32_t m_repetitions{};
            bool m_increase_depth{};
        };

        /** Returns false if the matching has failed */
        bool MatchCandidate(MatchingContext & i_context, Candidate & i_candidate)
        {
            const bool nest_index = i_candidate.m_repetitions != std::numeric_limits<uint32_t>::max();
            const uint32_t repetitions = nest_index ? i_candidate.m_repetitions : 1;

            size_t target_index = 0;
            for (uint32_t repetition = 0; repetition < repetitions; repetition++)
            {
                for (size_t pattern_index = 0; pattern_index < i_candidate.m_pattern.m_pattern.size(); target_index++, pattern_index++)
                {
                    const Tensor & pattern = i_candidate.m_pattern.m_pattern[pattern_index];

                    const ArgumentInfo & arg_info = i_candidate.m_pattern.m_arg_infos[pattern_index];

                    if (arg_info.m_cardinality.m_min != arg_info.m_cardinality.m_max)
                    {
                        size_t total_available_targets = i_candidate.m_target_arguments.size() - target_index;

                        size_t sub_pattern_count = pattern.GetExpression()->GetArguments().size();
                        DJUP_ASSERT(sub_pattern_count != 0); // empty repetitions are illegal and should raise an error when constructed

                        // compute usable range
                        Range usable;
                        usable.m_max = static_cast<int32_t>(total_available_targets - arg_info.m_remaining.m_min);
                        usable.m_min = static_cast<int32_t>(arg_info.m_remaining.m_max ==
                            std::numeric_limits<uint32_t>::max() ?
                            0 :
                            total_available_targets - arg_info.m_remaining.m_max);

                        usable = arg_info.m_cardinality.ClampRange(usable);

                        // align the usable range to be a multiple of sub_pattern_count
                        usable.m_min += static_cast<int32_t>(sub_pattern_count - 1);
                        usable.m_min -= usable.m_min % sub_pattern_count;
                        usable.m_max -= usable.m_max % sub_pattern_count;

                        const PatternInfo & pattern_info = GetPatternInfo(i_context, pattern);

                        uint32_t rep = NumericCast<uint32_t>(usable.m_min / sub_pattern_count);
                        for (size_t used = usable.m_min; used <= usable.m_max; used += sub_pattern_count, rep++)
                        {
                            DJUP_ASSERT(!nest_index); // repetitions can't be nested directly

                            LinearPath path(i_context, i_candidate);

                            // pre-pattern
                            PatternSegment pre_segment;
                            pre_segment.m_flags = pattern_info.m_flags;
                            pre_segment.m_pattern = pattern.GetExpression()->GetArguments();
                            pre_segment.m_arg_infos = pattern_info.m_arguments_info;
                            path.AddEdge(
                                i_candidate.m_target_arguments.subspan(target_index, used),
                                pre_segment, true, rep);

                            // post-pattern
                            PatternSegment post_segment;
                            post_segment.m_flags = pattern_info.m_flags;
                            post_segment.m_pattern = i_candidate.m_pattern.m_pattern.subspan(pattern_index + 1);
                            post_segment.m_arg_infos = i_candidate.m_pattern.m_arg_infos.subspan(pattern_index + 1);
                            path.AddEdge(
                                i_candidate.m_target_arguments.subspan(target_index + used),
                                post_segment);
                        }
                        return false;
                    }

                    if (target_index >= i_candidate.m_target_arguments.size())
                        return false;

                    const Tensor & target = i_candidate.m_target_arguments[target_index];

                    if (IsConstant(pattern))
                    {
                        if (!AlwaysEqual(pattern, target))
                            return false;
                    }
                    else if (IsIdentifier(pattern))
                    {
                        if (!i_context.m_namespace->TypeBelongsTo(
                            target.GetExpression()->GetType(),
                            pattern.GetExpression()->GetType()))
                            return false;

                        i_candidate.m_substitutions.push_back({ 
                            pattern.GetExpression()->GetName(), target });
                    }
                    else
                    {
                        if (pattern.GetExpression()->GetName() != target.GetExpression()->GetName())
                            return false;

                        // build pattern info
                        const PatternInfo & pattern_info = GetPatternInfo(i_context, pattern);

                        // if the target does not have enough arguments, early reject
                        size_t target_arguments = target.GetExpression()->GetArguments().size();
                        if (target_arguments >= pattern_info.m_arguments_range.m_min &&
                            target_arguments <= pattern_info.m_arguments_range.m_max)
                        {
                            LinearPath path(i_context, i_candidate);

                            // match content
                            path.AddEdge(target.GetExpression()->GetArguments(),
                                PatternSegment{ pattern_info.m_flags,
                                    pattern.GetExpression()->GetArguments(),
                                    pattern_info.m_arguments_info });

                            // rest of this repetition
                            const size_t remaining_in_pattern = i_candidate.m_pattern.m_pattern.size() - (pattern_index + 1);
                            path.AddEdge(i_candidate.m_target_arguments.subspan(target_index + 1, remaining_in_pattern),
                                PatternSegment{ pattern_info.m_flags,
                                    i_candidate.m_pattern.m_pattern.subspan(pattern_index + 1),
                                    i_candidate.m_pattern.m_arg_infos.subspan(pattern_index + 1) });

                            // remaining repetitions
                            const size_t target_start = target_index + 1 + remaining_in_pattern;
                            path.AddEdge(i_candidate.m_target_arguments.subspan(target_start),
                                i_candidate.m_pattern, false, repetitions - (repetition + 1));
                        }
                        return false;
                    }
                }
            }

            return true;
        }



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

