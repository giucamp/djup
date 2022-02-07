
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/pattern_match.h>
#include <private/builtin_names.h>
#include <private/substitute_by_predicate.h>
#include <private/expression.h>
#include <private/substitutions_tree.h>
#include <core/flags.h>
#include <core/diagnostic.h>
#include <vector>
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
    }

    Tensor SubstitutePatternMatch(const Tensor & i_source, const PatternMatch & i_match)
    {
        return SubstituteByPredicate(i_source, ApplySubstitutions{i_match});
    }

    constexpr uint32_t s_max_reps = std::numeric_limits<uint32_t>::max();

    // by default describes a non-variadic argument
    struct RepRange
    {
        uint32_t m_min = 1;
        uint32_t m_max = 1;
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
            uint32_t min = 0, max = 0;
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

    struct Candidate
    {
        Span<const Tensor> m_target_arguments;
        PatternSegment m_pattern;
        uint32_t m_repetitions = 0;
        bool m_has_repetitions = false;
        std::vector<VariadicIndex> m_variadic_indices;
    };

    StringBuilder & operator << (StringBuilder & i_dest, const Candidate & i_source)
    {
        i_dest << "Pattern: " << TensorListToString(i_source.m_pattern.m_pattern);
        if(i_source.m_repetitions != 0)
            i_dest << " (" << i_source.m_repetitions << " times)";
        i_dest.NewLine();
        i_dest << " Target: " << TensorListToString(i_source.m_target_arguments);
        return i_dest;
    }

    struct DisjunctiveTerm
    {
        std::vector<Candidate> m_candidates;
        SubstitutionsTree::NodeHandle m_substitution_node;
    };

    struct MatchingContext
    {
        std::vector<DisjunctiveTerm> m_disjunctive_terms;

        SubstitutionsTree m_substitutions_tree;

        std::unordered_map<const Expression*, PatternInfo> m_pattern_infos;
    };

    void DisjunctiveTermToString(StringBuilder & i_dest, const MatchingContext & i_context, const DisjunctiveTerm & i_source)
    {
        i_dest << "Disjunctive Term: ";
        if(i_context.m_substitutions_tree.IsValid(i_source.m_substitution_node))
            i_dest << "Node " << i_source.m_substitution_node.m_index;
        i_dest.NewLine();

        i_dest.Tab();
        for(const Candidate & candidate : i_source.m_candidates)
        {
            i_dest << candidate;
            i_dest.NewLine();
        }
        i_dest.Untab();
    }

    StringBuilder & operator << (StringBuilder & i_dest, const MatchingContext & i_source)
    {
        i_dest << "-----------------------------";
        i_dest << i_source.m_substitutions_tree;
        i_dest.NewLine();
        for(const DisjunctiveTerm & term : i_source.m_disjunctive_terms)
        {
            DisjunctiveTermToString(i_dest, i_source, term);
        }
        return i_dest;
    }

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

    /** Returns false if the matching has failed */
    bool MatchCandidate(MatchingContext & i_context, Candidate & i_candidate, SubstitutionsTree::NodeHandle i_subtitution_node)
    {
        const bool nest_index = i_candidate.m_has_repetitions;
        const uint32_t repetitions = nest_index ? i_candidate.m_repetitions : 1;
        
        if(nest_index)
            i_candidate.m_variadic_indices.push_back(VariadicIndex{0, repetitions});

        size_t target_index = 0;
        for(uint32_t repetition = 0; repetition < repetitions; repetition++)
        {
            if(nest_index)
                i_candidate.m_variadic_indices.back().m_index = repetition;
            
            for(size_t pattern_index = 0; pattern_index < i_candidate.m_pattern.m_pattern.size(); target_index++, pattern_index++)
            {
                const Tensor & pattern = i_candidate.m_pattern.m_pattern[pattern_index];

                if(i_candidate.m_pattern.m_ranges[pattern_index].m_min != i_candidate.m_pattern.m_ranges[pattern_index].m_max)
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

                    auto sub_node = i_context.m_substitutions_tree.NewNode(i_subtitution_node);

                    uint32_t rep = NumericCast<uint32_t>(min_usable / sub_pattern_count);
                    for(size_t used = min_usable; used <= max_usable; used += sub_pattern_count, rep++)
                    {
                        DisjunctiveTerm and_term;

                        assert(!nest_index); // repetitions can't be nested directly

                        // pre-pattern
                        Candidate new_candidate;
                        if(rep != 0)
                        {
                            new_candidate.m_pattern = { pattern.GetExpression()->GetArguments(),
                                pattern_info.m_pattern_arg_ranges,
                                pattern_info.m_pattern_arg_reiaming_ranges };
                            new_candidate.m_repetitions = rep;
                            new_candidate.m_has_repetitions = true;
                            new_candidate.m_variadic_indices = i_candidate.m_variadic_indices;
                            new_candidate.m_target_arguments = i_candidate.m_target_arguments.subspan(target_index, used);
                            and_term.m_candidates.push_back(std::move(new_candidate));
                        }

                        // post-pattern
                        new_candidate.m_pattern = { i_candidate.m_pattern.m_pattern.subspan(pattern_index + 1),
                            i_candidate.m_pattern.m_ranges.subspan(pattern_index + 1),
                            i_candidate.m_pattern.m_remaining.subspan(pattern_index + 1) };
                        new_candidate.m_repetitions = 0;
                        new_candidate.m_has_repetitions = false;
                        new_candidate.m_variadic_indices = i_candidate.m_variadic_indices;
                        new_candidate.m_target_arguments = i_candidate.m_target_arguments.subspan(target_index + used);
                        and_term.m_candidates.push_back(std::move(new_candidate));

                        and_term.m_substitution_node = i_context.m_substitutions_tree.NewNode(sub_node);
                        i_context.m_disjunctive_terms.push_back(std::move(and_term));
                    }
                    return true;
                }

                if(target_index >= i_candidate.m_target_arguments.size())
                    return false;

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

                    if(!i_context.m_substitutions_tree.AddSubstitution(i_subtitution_node, 
                            GetIdentifierName(pattern), i_candidate.m_variadic_indices, target))
                        return false; // incompatible substitution
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
                        new_candidate.m_pattern = { pattern.GetExpression()->GetArguments(),
                            pattern_info.m_pattern_arg_ranges,
                            pattern_info.m_pattern_arg_reiaming_ranges };
                        new_candidate.m_target_arguments = target.GetExpression()->GetArguments();
                        new_candidate.m_variadic_indices = i_candidate.m_variadic_indices;

                        DisjunctiveTerm disjunctive;
                        disjunctive.m_candidates.push_back(new_candidate);
                        disjunctive.m_substitution_node = i_subtitution_node;
                        i_context.m_disjunctive_terms.push_back(std::move(disjunctive));
                    }
                }
            }
        }

        return true;
    }

    
    void MatchDisjunctiveTerm(MatchingContext & i_context, DisjunctiveTerm & i_block)
    {
        if(i_context.m_substitutions_tree.IsValid(i_block.m_substitution_node))
        { 
            for(Candidate & candidate : i_block.m_candidates)
            {
                if(!MatchCandidate(i_context, candidate, i_block.m_substitution_node))
                {
                    i_context.m_substitutions_tree.DeleteNode(i_block.m_substitution_node);
                    break;
                }
            }
        }
    }

    std::vector<PatternMatch> Match(const Tensor & i_target, const Tensor & i_pattern)
    {
        MatchingContext context;
        RepRange single_range = {1, 1};
        RepRange single_remaining = {0, 0};

        Candidate root_candidate;
        root_candidate.m_target_arguments = {&i_target, 1};
        root_candidate.m_pattern = {{&i_pattern, 1}, {&single_range, 1}, {&single_remaining, 1}};

        DisjunctiveTerm root_term;
        root_term.m_substitution_node = context.m_substitutions_tree.GetRoot();
        root_term.m_candidates.push_back(std::move(root_candidate));

        context.m_disjunctive_terms.push_back(std::move(root_term));

        int dbg_step = 0;
        do {

            #if DBG_PATTERN_TRACE
                PrintLn();
                Print(" ------ Step ", dbg_step, " ");
                PrintLn(context);
            #endif

            DisjunctiveTerm term = std::move(context.m_disjunctive_terms.back());
            context.m_disjunctive_terms.pop_back();
            MatchDisjunctiveTerm(context, term);

            dbg_step++;

        } while(!context.m_disjunctive_terms.empty());

        #if DBG_PATTERN_TRACE
            PrintLn();
            Print(" ------ Final State ");
            PrintLn(context);
        #endif

        std::vector<PatternMatch> matches = context.m_substitutions_tree.GetMathes();

        return matches;
    }
}
