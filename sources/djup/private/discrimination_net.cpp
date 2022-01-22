
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/discrimination_net.h>
#include <private/builtin_names.h>
#include <core/flags.h>
#include <vector>
#include <private/substitute_by_predicate.h>

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

    /** Immutable flat representation of an expression.
        An expression is a directed acyclic graph. The construction of a flat representation 
        adds an overhead cost, but it greatly simplifies the pattern matching algorithms. */
    class DiscriminationNet::LinearizedExpression
    {
    public:

        enum Flags
        {
            None = 0,
            GroupConstantExpression = 1 << 0, /** The arguments of constant expression are not expanded */
        };

        LinearizedExpression(const Tensor & i_target, Flags i_flags)
        {
            Linearize(i_target, i_flags);
        }

        size_t GetLength() const
        {
            return m_tokens.size();
        }

        const Tensor & GetToken(size_t i_index) const
        {
            return m_tokens[i_index].m_expr;
        }

        uint32_t GetSibilingOffset(size_t i_index) const
        {
            return m_tokens[i_index].m_sibling_offset;
        }

        bool BeginsArguments(size_t i_index) const
        {
            return m_tokens[i_index].m_begin_arguments;
        }

        bool EndsArguments(size_t i_index) const
        {
            return m_tokens[i_index].m_end_arguments;
        }

    private:

        void Linearize(const Tensor & i_target, Flags i_flags)
        {
            const size_t token_index = m_tokens.size();
            m_tokens.push_back(Token{i_target});

            bool expand = !NameIs(i_target, builtin_names::Identifier)
                && !i_target.GetExpression()->GetArguments().empty();

            if(IsConstant(i_target) && HasFlag(i_flags, Flags::GroupConstantExpression))
                expand = false;

            if(expand)
            {
                m_tokens.back().m_begin_arguments = true;

                for(const Tensor & argument : i_target.GetExpression()->GetArguments())
                    Linearize(argument, i_flags);

                m_tokens.emplace_back();
                m_tokens.back().m_end_arguments = true;
            }

            m_tokens[token_index].m_sibling_offset = NumericCast<uint32_t>(m_tokens.size() - token_index);
        }

        struct Token
        {
            Tensor m_expr;
            uint32_t m_sibling_offset = 0;
            bool m_begin_arguments = false;
            bool m_end_arguments = false;
        };
        std::vector<Token> m_tokens;
    };

    DiscriminationNet::DiscriminationNet() = default;

    void DiscriminationNet::AddPattern(size_t i_pattern_id, 
        const Tensor & i_pattern, const Tensor & i_condition)
    {
        LinearizedExpression pattern(i_pattern, LinearizedExpression::Flags::GroupConstantExpression);

        const size_t pattern_length = pattern.GetLength();
        size_t prev_node = 0;

        for(size_t token_index = 0; token_index < pattern_length; token_index++)
        {
            const Tensor & token = pattern.GetToken(token_index);

            Edge edge;
            edge.m_begin_arguments = pattern.BeginsArguments(token_index);
            edge.m_end_arguments = pattern.EndsArguments(token_index);
            edge.m_is_terminal = token_index + 1 >= pattern_length;
            edge.m_expr = token;

            size_t dest_node = m_next_node_index++;
            edge.m_dest_node = dest_node;

            m_edges.insert(std::make_pair(prev_node, std::move(edge)));
            prev_node = dest_node;
        }

        m_terminal_states.insert(std::make_pair(prev_node, i_pattern_id));
    }

    struct DiscriminationNet::WalkingHead
    {
        size_t m_source_node = 0;
        size_t m_current_token = 0;
        std::unordered_map<const Expression*, Tensor> m_substitutions;
        std::unordered_map<const Expression*, size_t> m_expansions;
    };

    struct Candidate
    {
        Span<const Tensor> m_pattern_arguments;
        Span<const Tensor> m_target_arguments;
        size_t m_variadic_arguments_count = {};
        size_t m_repetitions = {};
        std::unordered_map<const Expression*, Tensor> m_substitutions;
        std::unordered_map<const Expression*, size_t> m_expansions;
    };

    struct MatchingContext
    {
        std::vector<Candidate> m_candidates;
        std::vector<PatternMatch> m_matches;
    };

    bool AddSubstitution(Candidate & i_dest, const Tensor & i_pattern_var, const Tensor & i_value);
    std::vector<PatternMatch> Match(const Tensor & i_target, const Tensor & i_pattern);
    bool MatchArguments(MatchingContext & i_context, Candidate i_candidate);

    bool AddSubstitution(Candidate & i_dest, const Tensor & i_pattern_var, const Tensor & i_value)
    {
        auto res = i_dest.m_substitutions.insert({i_pattern_var.GetExpression().get(), i_value});
        if(res.second)
            return true;

        // already present, enforce coherence
        return AlwaysEqual(i_value, res.first->second);
    }

    std::vector<PatternMatch> Match(const Tensor & i_target, const Tensor & i_pattern)
    {
        MatchingContext context;
        Candidate candidate;
        candidate.m_pattern_arguments = {&i_pattern, 1};
        candidate.m_target_arguments = {&i_target, 1};
        candidate.m_repetitions = 1;
        MatchArguments(context, candidate);
        return std::move(context.m_matches);
    }

    /** */
    bool MatchArguments(
        MatchingContext & i_context,
        Candidate i_candidate)
    {
        size_t target_arg_index = 0;

        for(size_t repetition_index = 0; repetition_index < i_candidate.m_repetitions; repetition_index++)
            for(size_t arg_index = 0; arg_index < i_candidate.m_pattern_arguments.size(); target_arg_index++, arg_index++)
        {
            const Tensor & pattern_arg = i_candidate.m_pattern_arguments[arg_index];
            const Tensor & target_arg = i_candidate.m_target_arguments[target_arg_index];
            
            if(IsConstant(pattern_arg))
            {
                if(!AlwaysEqual(pattern_arg, target_arg))
                    return false;
            }
            else if(NameIs(pattern_arg, builtin_names::Identifier))
            {
                if(!Is(target_arg, pattern_arg))
                    return false; // type mismatch
                
                if(!AddSubstitution(i_candidate, pattern_arg, target_arg))
                    return false; // incompatible substitution
            }
            else if(NameIs(pattern_arg, builtin_names::RepetitionsZeroToMany))
            {
                size_t min_rep = 0;
                size_t max_rep = i_candidate.m_variadic_arguments_count / pattern_arg.GetExpression()->GetArguments().size();

                for(size_t rep = min_rep; rep < max_rep; rep++)
                {
                    Candidate new_candidate = i_candidate;
                    new_candidate.m_repetitions = rep;
                    new_candidate.m_variadic_arguments_count -= (rep - min_rep);
                    new_candidate.m_pattern_arguments = pattern_arg.GetExpression()->GetArguments();
                    new_candidate.m_target_arguments = i_candidate.m_pattern_arguments.subspan(arg_index + 1);
                    i_context.m_candidates.push_back(std::move(new_candidate));
                }

                return false;
            }
            else if(NameIs(pattern_arg, builtin_names::RepetitionsZeroToOne))
            {

            }
            else if(NameIs(pattern_arg, builtin_names::RepetitionsOneToMany))
            {

            }
            else
            {
                if(pattern_arg.GetExpression()->GetName() != target_arg.GetExpression()->GetName())
                    return false;

                size_t non_variadic_symbols = 0;
                for(const Tensor & arg : pattern_arg.GetExpression()->GetArguments())
                {
                    if(NameIs(arg, builtin_names::RepetitionsZeroToMany))
                        ;
                    else if(NameIs(arg, builtin_names::RepetitionsZeroToOne))
                        ;
                    else if(NameIs(arg, builtin_names::RepetitionsOneToMany))
                        non_variadic_symbols++;
                    else
                        non_variadic_symbols++;
                }

                // is the target does not have enough arguments, early reject
                size_t target_arguments = target_arg.GetExpression()->GetArguments().size();
                if(non_variadic_symbols <= target_arguments)
                {
                    // total number of arguments that can be distributed to variadic expressions
                    size_t variadic_arguments_count = target_arguments - non_variadic_symbols;

                    Candidate new_candidate = i_candidate;
                    new_candidate.m_pattern_arguments = pattern_arg.GetExpression()->GetArguments();
                    new_candidate.m_target_arguments = target_arg.GetExpression()->GetArguments();
                    new_candidate.m_repetitions = 1;
                    new_candidate.m_variadic_arguments_count = variadic_arguments_count;
                    MatchArguments(i_context, std::move(new_candidate));
                }
            }
        }

        return true;
    }



    void DiscriminationNet::FindMatches(const Tensor & i_target, std::vector<PatternMatch> & o_matches) const
    {
        LinearizedExpression target(i_target, LinearizedExpression::Flags::None);

        std::vector<WalkingHead> heads;
        heads.emplace_back(WalkingHead{0, 0});

        while(!heads.empty())
        {
            WalkingHead curr_head = std::move(heads.back());
            heads.pop_back();

            if(curr_head.m_current_token >= target.GetLength())
                break;
            const Tensor & token = target.GetToken(curr_head.m_current_token);

            auto edges = m_edges.equal_range(curr_head.m_source_node);
            for(auto it = edges.first; it != edges.second; ++it)
            {
                const Edge & edge = it->second;

                if(IsConstant(edge.m_expr))
                {
                    if(!AlwaysEqual(token, edge.m_expr))
                        continue;

                    curr_head.m_current_token += target.GetSibilingOffset(curr_head.m_current_token);
                }
                else if(NameIs(edge.m_expr, builtin_names::Identifier))
                {
                    if(!Is(token, edge.m_expr))
                        continue;
                    
                    curr_head.m_substitutions.emplace(std::pair{edge.m_expr.GetExpression().get(), token});
                    curr_head.m_current_token += target.GetSibilingOffset(curr_head.m_current_token);
                }
                else
                {
                    if(token.GetExpression()->GetName() != edge.m_expr.GetExpression()->GetName())
                        continue;
                    curr_head.m_current_token++;
                }

                if(edge.m_is_terminal)
                {
                    auto it = m_terminal_states.find(edge.m_dest_node);
                    if(it == m_terminal_states.end())
                        Error("DiscriminationNet: terminal state not found");
                    o_matches.push_back(PatternMatch{it->second, curr_head.m_substitutions});
                }

                curr_head.m_source_node = edge.m_dest_node;
                heads.push_back(std::move(curr_head));
            }
        }
    }
}
