
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/pattern_match.h>
#include <private/builtin_names.h>
#include <private/substitute_by_predicate.h>
#include <private/expression.h>
#include <core/flags.h>
#include <vector>
#include <limits>
#include <unordered_map>

namespace djup
{
    namespace
    {
        struct ApplySubstitutions
        {
            const PatternMatch & m_match;

            Tensor operator () (const Tensor & i_candidate) const
            {
                // substitution
                {
                    auto it = m_match.m_substitutions.find(i_candidate.GetExpression().get());
                    if(it != m_match.m_substitutions.end())
                        return it->second;
                }

                // argument expansion
                std::vector<Tensor> new_arguments;
                for(const Tensor & argument : i_candidate.GetExpression()->GetArguments())
                {
                    auto it = m_match.m_expansions.find(argument.GetExpression().get());
                    if(it != m_match.m_expansions.end())
                    {
                        
                    }
                }
                return i_candidate;
            }
        };
    }

    Tensor SubstitutePatternMatch(const Tensor & i_source, const PatternMatch & i_match)
    {
        return SubstituteByPredicate(i_source, ApplySubstitutions{i_match});
    }

 
    constexpr size_t s_max_reps = std::numeric_limits<size_t>::max();

    // by default describes a non-variadic argument
    struct RepRange
    {
        size_t m_min = 1;
        size_t m_max = 1;
    };

    struct PatternInfo
    {
        size_t m_non_variadic_symbols{};
        std::vector<RepRange> m_pattern_arg_ranges;
        std::vector<RepRange> m_pattern_arg_reiaming_ranges;
    };

    PatternInfo BuildPatternInfo(Span<const Tensor> i_pattern_args)
    {
        PatternInfo result;

        // fill m_pattern_arg_ranges
        result.m_pattern_arg_ranges.resize(i_pattern_args.size());
        for(size_t sub_pattern_index = 0; sub_pattern_index < i_pattern_args.size(); sub_pattern_index++)
        {
            const Tensor & arg = i_pattern_args[sub_pattern_index]; 

            RepRange & arg_range = result.m_pattern_arg_ranges[sub_pattern_index];

            if(NameIs(arg, builtin_names::RepetitionsZeroToMany))
            {
                arg_range.m_min = 0;
                arg_range.m_max = s_max_reps;
            }
            else if(NameIs(arg, builtin_names::RepetitionsZeroToOne))
            {
                arg_range.m_min = 0;
                arg_range.m_max = 1;
            }
            else if(NameIs(arg, builtin_names::RepetitionsOneToMany))
            {
                arg_range.m_min = 1;
                arg_range.m_max = s_max_reps;
                result.m_non_variadic_symbols++;
            }
            else
            {
                result.m_non_variadic_symbols++;
            }
        }

        // fill m_pattern_arg_reiaming_ranges
        result.m_pattern_arg_reiaming_ranges.resize(i_pattern_args.size());
        for(size_t sub_pattern_index = 0; sub_pattern_index < i_pattern_args.size(); sub_pattern_index++)
        {
            size_t min = 0, max = 0;
            for(size_t j = sub_pattern_index + 1; j < i_pattern_args.size(); j++)
            {
                min += result.m_pattern_arg_ranges[j].m_min;

                auto new_max = max + result.m_pattern_arg_ranges[j].m_max;
                if(max <= new_max)
                    max = new_max;
                else
                    max = s_max_reps; // overflow, max or m_max were s_max_reps
            }
            result.m_pattern_arg_reiaming_ranges[sub_pattern_index].m_min = min;
            result.m_pattern_arg_reiaming_ranges[sub_pattern_index].m_max = max;
        }

        return result;
    }

    struct MathingState
    {
        std::unordered_map<const Expression*, Tensor> m_substitutions;
        std::unordered_map<const Expression*, size_t> m_expansions;
    };

    struct Candidate
    {
        Span<const Tensor> m_target_arguments;

        size_t m_pre_pattern_repetitions = 0;
        Span<const Tensor> m_pre_pattern;
        Span<const RepRange> m_pre_pattern_ranges;
        Span<const RepRange> m_pre_pattern_remaining_ranges;

        Span<const Tensor> m_post_pattern;
        Span<const RepRange> m_post_pattern_ranges;
        Span<const RepRange> m_post_pattern_remaining_ranges;

        MathingState m_state;
    };

    struct MatchingContext
    {
        std::vector<Candidate> m_candidates;
        std::vector<PatternMatch> m_matches;

        std::unordered_map<const Expression*, PatternInfo> m_pattern_infos;
    };

    const PatternInfo & GetPatternInfo(MatchingContext & i_context, const Tensor & i_pattern)
    {
        const Expression * expr = i_pattern.GetExpression().get();
        auto it = i_context.m_pattern_infos.find(expr);
        if(it != i_context.m_pattern_infos.end())
            return it->second;
        auto res = i_context.m_pattern_infos.insert({expr, BuildPatternInfo(expr->GetArguments())});
        assert(res.second);
        return res.first->second;
    }

    bool MatchArguments(MatchingContext & i_context, Candidate & i_candidate);

    bool AddSubstitution(MathingState & i_dest, const Tensor & i_pattern_var, const Tensor & i_value)
    {
        auto res = i_dest.m_substitutions.insert({i_pattern_var.GetExpression().get(), i_value});
        if(res.second)
            return true;

        // already present, enforce coherence
        return AlwaysEqual(i_value, res.first->second);
    }

    bool MatchFlatSeq(
        Span<const Tensor> i_targets,
        Span<const Tensor> i_patterns,
        Span<const RepRange> i_pattern_ranges,
        Span<const RepRange> i_pattern_remaining_ranges,
        size_t & io_target_index,
        MathingState & i_state,
        MatchingContext & i_context
        )
    {
        size_t & target_index = io_target_index;
        for(size_t pattern_index = 0; pattern_index < i_patterns.size(); target_index++, pattern_index++)
        {
            const Tensor & pattern = i_patterns[pattern_index];
            const Tensor & target = i_targets[target_index];

            if(IsConstant(pattern))
            {
                if(!AlwaysEqual(pattern, target))
                    return false;
            }
            else if(NameIs(pattern, builtin_names::Identifier))
            {
                if(!Is(target, pattern))
                    return false; // type mismatch

                if(!AddSubstitution(i_state, pattern, target))
                    return false; // incompatible substitution
            }
            else if(NameIs(pattern, builtin_names::RepetitionsZeroToMany) ||
                NameIs(pattern, builtin_names::RepetitionsZeroToOne) ||
                NameIs(pattern, builtin_names::RepetitionsOneToMany))
            {
                size_t total_available_targets = i_targets.size() - target_index;

                size_t sub_pattern_count = pattern.GetExpression()->GetArguments().size();
                assert(sub_pattern_count != 0); // empty repetitions are illegal and should raise an error when constructed

                // compute usable range
                size_t max_usable = total_available_targets - i_pattern_remaining_ranges[pattern_index].m_min;
                size_t min_usable = i_pattern_remaining_ranges[pattern_index].m_max == s_max_reps ?
                    0 :
                    total_available_targets - i_pattern_remaining_ranges[pattern_index].m_max;
                if(max_usable > i_pattern_ranges[pattern_index].m_max)
                    max_usable = i_pattern_ranges[pattern_index].m_max;
                if(min_usable < i_pattern_ranges[pattern_index].m_min)
                    min_usable = i_pattern_ranges[pattern_index].m_min;

                // align the usable range to be a multiple of sub_pattern_count
                min_usable += sub_pattern_count - 1;
                min_usable -= min_usable % sub_pattern_count;
                max_usable -= max_usable % sub_pattern_count;

                size_t rep = min_usable / sub_pattern_count;
                for(size_t used = min_usable; used <= max_usable; used += sub_pattern_count, rep++)
                {
                    Candidate new_candidate;
                    new_candidate.m_state = i_state;
                    new_candidate.m_state.m_expansions.insert({pattern.GetExpression().get(), rep});
                    new_candidate.m_pre_pattern_repetitions = rep;
                    new_candidate.m_pre_pattern = pattern.GetExpression()->GetArguments();
                    new_candidate.m_post_pattern = i_patterns.subspan(pattern_index + 1);
                    new_candidate.m_target_arguments = i_targets.subspan(target_index);
                    i_context.m_candidates.push_back(std::move(new_candidate));
                }

                return false;
            }
            else
            {
                if(pattern.GetExpression()->GetName() != target.GetExpression()->GetName())
                    return false;

                Span<const Tensor> sub_patterns = pattern.GetExpression()->GetArguments();

                // build pattern info
                const PatternInfo & pattern_info = GetPatternInfo(i_context, pattern);

                // if the target does not have enough arguments, early reject
                size_t target_arguments = target.GetExpression()->GetArguments().size();
                if(pattern_info.m_non_variadic_symbols <= target_arguments)
                {
                    // total number of arguments that can be distributed to variadic expressions
                    Candidate new_candidate;
                    new_candidate.m_state = i_state;
                    new_candidate.m_post_pattern = pattern.GetExpression()->GetArguments();
                    new_candidate.m_post_pattern_ranges = pattern_info.m_pattern_arg_ranges;
                    new_candidate.m_post_pattern_remaining_ranges = pattern_info.m_pattern_arg_reiaming_ranges;
                    new_candidate.m_target_arguments = target.GetExpression()->GetArguments();
                    i_context.m_candidates.push_back(new_candidate);
                }
                
                return false;
            }
        }

        return true;
    }

    /** */
    bool MatchArguments(
        MatchingContext & i_context,
        Candidate & i_candidate)
    {
        size_t target_arg_index = 0;

        for(size_t repetition_index = 0; repetition_index < i_candidate.m_pre_pattern_repetitions; repetition_index++)
        {
            if(!MatchFlatSeq(i_candidate.m_target_arguments, i_candidate.m_pre_pattern, 
                    i_candidate.m_pre_pattern_ranges, i_candidate.m_pre_pattern_remaining_ranges,
                    target_arg_index, i_candidate.m_state, i_context))
            {
                return false;
            }
        }

        if(!MatchFlatSeq(i_candidate.m_target_arguments, i_candidate.m_post_pattern, 
            i_candidate.m_post_pattern_ranges, i_candidate.m_post_pattern_remaining_ranges,
            target_arg_index, i_candidate.m_state, i_context))
        {
            return false;
        }

        return true;
    }

    std::vector<PatternMatch> Match(const Tensor & i_target, const Tensor & i_pattern)
    {
        MatchingContext context;
        {
            Candidate candidate;
            candidate.m_post_pattern = {&i_pattern, 1};
            candidate.m_target_arguments = {&i_target, 1};
            context.m_candidates.push_back(std::move(candidate));
        }

        while(!context.m_candidates.empty())
        {
            Candidate candidate = std::move(context.m_candidates.back());
            context.m_candidates.pop_back();
            if(MatchArguments(context, candidate))
            {
                context.m_matches.push_back(PatternMatch{0,
                    std::move(candidate.m_state.m_substitutions), 
                    std::move(candidate.m_state.m_expansions)});
            }
        };

        return std::move(context.m_matches);
    }
}
