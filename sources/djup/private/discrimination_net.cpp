
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/discrimination_net.h>
#include <private/builtin_names.h>
#include <private/substitute_by_predicate.h>
#include <private/expression.h>
#include <core/flags.h>
#include <vector>
#include <limits>

namespace djup
{

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
