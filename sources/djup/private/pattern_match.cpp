
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/pattern_match.h>
#include <private/builtin_names.h>
#include <private/substitute_by_predicate.h>
#include <private/expression.h>
#include <core/flags.h>
#include <core/diagnostic.h>
#include <vector>
#include <limits>
#include <unordered_map>

namespace djup
{
    #define DBG_PATTERN_TRACE 0

    namespace
    {
        struct ApplySubstitutions
        {
            const PatternMatch & m_match;

            Tensor operator () (const Tensor & i_candidate) const
            {
                // substitution
                /*{
                    auto it = m_match.m_substitutions.find({i_candidate.GetExpression()->GetName(), 0});
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
                }*/
                return i_candidate;
            }
        };

        #if DBG_PATTERN_TRACE
            std::string TensorListToString(Span<const Tensor> i_tensors)
            {
                std::string result;
                for(size_t i = 0; i < i_tensors.size(); i++)
                {
                    if(i)
                        result += ", ";
                    result += ToSimplifiedStringForm(i_tensors[i]);
                }
                return result;
            }
        #endif
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
        size_t m_min_arguments{}, m_max_arguments{};
        std::vector<RepRange> m_pattern_arg_ranges;
        std::vector<RepRange> m_pattern_arg_reiaming_ranges;
    };

    struct PatternSegment
    {
        Span<const Tensor> m_pattern;
        Span<const RepRange> m_ranges;
        Span<const RepRange> m_remaining;

        PatternSegment() = default;

        PatternSegment(Span<const Tensor> i_pattern, Span<const RepRange> i_ranges, Span<const RepRange> i_remaining)
            : m_pattern(i_pattern), m_ranges(i_ranges), m_remaining(i_remaining)
        {
            assert(m_ranges.size() == m_pattern.size());
            assert(m_ranges.size() == m_remaining.size());
        }
    };

    PatternInfo BuildPatternInfo(Span<const Tensor> i_pattern_args)
    {
        PatternInfo result;

        // fill m_pattern_arg_ranges
        result.m_pattern_arg_ranges.resize(i_pattern_args.size());
        bool upper_unbounded = false;
        for(size_t sub_pattern_index = 0; sub_pattern_index < i_pattern_args.size(); sub_pattern_index++)
        {
            const Tensor & arg = i_pattern_args[sub_pattern_index]; 

            RepRange & arg_range = result.m_pattern_arg_ranges[sub_pattern_index];

            if(NameIs(arg, builtin_names::RepetitionsZeroToMany))
            {
                arg_range.m_min = 0;
                arg_range.m_max = s_max_reps;
                upper_unbounded = true;
            }
            else if(NameIs(arg, builtin_names::RepetitionsZeroToOne))
            {
                arg_range.m_min = 0;
                arg_range.m_max = 1;
                result.m_max_arguments++;
            }
            else if(NameIs(arg, builtin_names::RepetitionsOneToMany))
            {
                arg_range.m_min = 1;
                arg_range.m_max = s_max_reps;
                result.m_min_arguments++;
                upper_unbounded = true;
            }
            else
            {
                result.m_min_arguments++;
                result.m_max_arguments++;
            }
        }
        if(upper_unbounded)
            result.m_max_arguments = s_max_reps;

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
        std::unordered_map<Name, PatternMatch::VariableValue> m_substitutions;
    };

    struct Candidate
    {
        Span<const Tensor> m_target_arguments;

        std::vector<size_t> m_variadic_indices;

        size_t m_repetitions = 1;
        bool m_add_varadic_index = false;

        // if this candidate fails, also the parent fails. Zero means "Root Candidate", otherwise it's the index incremented
        // The list of candidates is a stack, so this index can't be invalidated before this Candidate is removed
        // Anyway this prevents parallelization
        size_t m_inc_parent_index = 0;
        
        PatternSegment m_pattern;
        
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

    bool AddSubstitution(MathingState & i_dest, const Tensor & i_pattern_variable,
        Span<const size_t> i_variadic_indices, const Tensor & i_value)
    {
        #if DBG_PATTERN_TRACE
            PrintLn("AddSubstitution: ", GetIdentifierName(i_pattern_variable), "[", i_variadic_indices, "]");
        #endif
        
        PatternMatch::VariableValue * value = &i_dest.m_substitutions[GetIdentifierName(i_pattern_variable)];

        for(size_t index : i_variadic_indices)
        {
            if(std::holds_alternative<std::monostate>(value->m_value))
                value->m_value = std::vector<PatternMatch::VariableValue>{};

            auto & values = std::get<std::vector<PatternMatch::VariableValue>>(value->m_value);
            if(index >= values.size())
                values.resize(index + 1);

            values[index] = PatternMatch::VariableValue{};
            value = &values[index];
        }

        if(!std::holds_alternative<std::monostate>(value->m_value))
            return AlwaysEqual(std::get<Tensor>(value->m_value), i_value);
        
        return true;
    }

    bool MatchArguments(Candidate & i_candidate, MatchingContext & i_context)
    {
        #if DBG_PATTERN_TRACE
            PrintLn("MatchArguments\n\ttarget: ", TensorListToString(i_candidate.m_target_arguments),
                "\n\tpattern: ", TensorListToString(i_candidate.m_pattern.m_pattern),
                "\n\trepetitions: ", i_candidate.m_repetitions, ", variadic: ", i_candidate.m_add_varadic_index);
        #endif

        size_t target_index = 0;
        for(size_t repetition = 0; repetition < i_candidate.m_repetitions; repetition++)
        {
            if(i_candidate.m_add_varadic_index)
                i_candidate.m_variadic_indices.push_back(repetition);

            for(size_t pattern_index = 0; pattern_index < i_candidate.m_pattern.m_pattern.size(); target_index++, pattern_index++)
            {
                const Tensor & pattern = i_candidate.m_pattern.m_pattern[pattern_index];
                const Tensor & target = i_candidate.m_target_arguments[target_index];

                if(IsConstant(pattern))
                {
                    if(!AlwaysEqual(pattern, target))
                        return false;
                }
                else if(NameIs(pattern, builtin_names::Identifier))
                {
                    if(!Is(target, pattern))
                        return false; // type mismatch

                    if(!AddSubstitution(i_candidate.m_state, pattern, i_candidate.m_variadic_indices, target))
                        return false; // incompatible substitution
                }
                else if(i_candidate.m_pattern.m_ranges[pattern_index].m_min != i_candidate.m_pattern.m_ranges[pattern_index].m_max)
                {
                    assert(NameIs(pattern, builtin_names::RepetitionsZeroToMany) ||
                        NameIs(pattern, builtin_names::RepetitionsZeroToOne) ||
                        NameIs(pattern, builtin_names::RepetitionsOneToMany));

                    size_t total_available_targets = i_candidate.m_target_arguments.size() - target_index;

                    size_t sub_pattern_count = pattern.GetExpression()->GetArguments().size();
                    assert(sub_pattern_count != 0); // empty repetitions are illegal and should raise an error when constructed

                    // compute usable range
                    size_t max_usable = total_available_targets - i_candidate.m_pattern.m_remaining[pattern_index].m_min;
                    size_t min_usable = i_candidate.m_pattern.m_remaining[pattern_index].m_max == s_max_reps ?
                        0 :
                        total_available_targets - i_candidate.m_pattern.m_remaining[pattern_index].m_max;
                    if(max_usable > i_candidate.m_pattern.m_ranges[pattern_index].m_max)
                        max_usable = i_candidate.m_pattern.m_ranges[pattern_index].m_max;
                    if(min_usable < i_candidate.m_pattern.m_ranges[pattern_index].m_min)
                        min_usable = i_candidate.m_pattern.m_ranges[pattern_index].m_min;

                    // align the usable range to be a multiple of sub_pattern_count
                    min_usable += sub_pattern_count - 1;
                    min_usable -= min_usable % sub_pattern_count;
                    max_usable -= max_usable % sub_pattern_count;

                    const PatternInfo & pattern_info = GetPatternInfo(i_context, pattern);

                    size_t rep = min_usable / sub_pattern_count;
                    for(size_t used = min_usable; used <= max_usable; used += sub_pattern_count, rep++)
                    {
                        // post-pattern
                        Candidate new_candidate;
                        new_candidate.m_state = i_candidate.m_state;
                        new_candidate.m_inc_parent_index = i_candidate.m_inc_parent_index;
                        new_candidate.m_pattern = { i_candidate.m_pattern.m_pattern.subspan(pattern_index + 1),
                            i_candidate.m_pattern.m_ranges.subspan(pattern_index + 1),
                            i_candidate.m_pattern.m_remaining.subspan(pattern_index + 1) };
                        new_candidate.m_variadic_indices = i_candidate.m_variadic_indices;
                        new_candidate.m_target_arguments = i_candidate.m_target_arguments.subspan(target_index + used);
                        i_context.m_candidates.push_back(std::move(new_candidate));

                        // pre-pattern
                        new_candidate.m_state = i_candidate.m_state;
                        new_candidate.m_inc_parent_index = i_context.m_candidates.size(); // index of the post-pattern (incremented)
                        new_candidate.m_pattern = { pattern.GetExpression()->GetArguments(),
                            pattern_info.m_pattern_arg_ranges,
                            pattern_info.m_pattern_arg_reiaming_ranges };
                        new_candidate.m_repetitions = rep;
                        new_candidate.m_add_varadic_index = true;
                        new_candidate.m_variadic_indices = i_candidate.m_variadic_indices;
                        new_candidate.m_target_arguments = i_candidate.m_target_arguments.subspan(target_index, used);
                        i_context.m_candidates.push_back(std::move(new_candidate));
                    }

                    i_candidate.m_inc_parent_index = 0;
                    return false;
                }
                else
                {
                    if(pattern.GetExpression()->GetName() != target.GetExpression()->GetName())
                        return false;

                    // build pattern info
                    const PatternInfo & pattern_info = GetPatternInfo(i_context, pattern);

                    // if the target does not have enough arguments, early reject
                    size_t target_arguments = target.GetExpression()->GetArguments().size();
                    if(target_arguments >= pattern_info.m_min_arguments &&
                        target_arguments <= pattern_info.m_max_arguments )
                    {
                        Candidate new_candidate;
                        new_candidate.m_state = i_candidate.m_state;
                        new_candidate.m_inc_parent_index = i_candidate.m_inc_parent_index;
                        new_candidate.m_variadic_indices = i_candidate.m_variadic_indices;
                        new_candidate.m_pattern = { pattern.GetExpression()->GetArguments(),
                            pattern_info.m_pattern_arg_ranges,
                            pattern_info.m_pattern_arg_reiaming_ranges };
                        new_candidate.m_target_arguments = target.GetExpression()->GetArguments();
                        i_context.m_candidates.push_back(new_candidate);
                    }
                
                    i_candidate.m_inc_parent_index = 0;
                    return false;
                }
            }

            if(i_candidate.m_add_varadic_index)
                i_candidate.m_variadic_indices.pop_back();
        }

        #if DBG_PATTERN_TRACE
            PrintLn("MATCH:\n\ttarget: ", TensorListToString(i_candidate.m_target_arguments),
                "\n\tpattern: ", TensorListToString(i_candidate.m_pattern.m_pattern));
        #endif

        return true;
    }

    std::vector<PatternMatch> Match(const Tensor & i_target, const Tensor & i_pattern)
    {
        MatchingContext context;
        RepRange single_range = {1, 1};
        RepRange single_remaining = {0, 0};
        PatternInfo pattern_info = BuildPatternInfo(i_pattern.GetExpression()->GetArguments());
        {
            Candidate candidate;
            candidate.m_target_arguments = {&i_target, 1};
            candidate.m_pattern = {{&i_pattern, 1}, {&single_range, 1}, {&single_remaining, 1}};
            context.m_candidates.push_back(std::move(candidate));
        }

        while(!context.m_candidates.empty())
        {
            Candidate candidate = std::move(context.m_candidates.back());
            context.m_candidates.pop_back();
            if(MatchArguments(candidate, context))
            {
                if(candidate.m_inc_parent_index != 0)
                {
                    Candidate & parent = context.m_candidates[candidate.m_inc_parent_index - 1];
                    for(auto & subst : candidate.m_state.m_substitutions)
                        parent.m_state.m_substitutions.insert(std::move(subst));
                }
                else
                {
                    context.m_matches.push_back(PatternMatch{0,
                        std::move(candidate.m_state.m_substitutions)});
                }
            }
            else if(candidate.m_inc_parent_index != 0)
            {
                context.m_candidates.erase(context.m_candidates.begin() + (candidate.m_inc_parent_index - 1));
            }
        };

        return std::move(context.m_matches);
    }
}
