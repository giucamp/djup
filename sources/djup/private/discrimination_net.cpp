
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/discrimination_net.h>
#include <private/builtin_names.h>
#include <vector>

namespace djup
{
    /** Immutable flat representation of an expression.
        An expression is a directed acyclic graph. The construction of a flat representation 
        adds an overhead cost, but it greatly simplifies the pattern matching algorithms. */
    class DiscriminationNet::LinearizedExpression
    {
    public:

        LinearizedExpression(const Tensor & i_target)
        {
            Linearize(i_target);
        }

        size_t GetLength() const
        {
            return m_tokens.size();
        }

        const Tensor & GetToken(size_t i_index) const
        {
            return m_tokens[i_index].m_expr;
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

        void Linearize(const Tensor & i_target)
        {
            m_tokens.push_back(Token{i_target});

            if(!NameIs(i_target, builtin_names::Identifier)
                && !i_target.GetExpression()->GetArguments().empty())
            {
                m_tokens.back().m_begin_arguments = true;

                for(const Tensor & argument : i_target.GetExpression()->GetArguments())
                    Linearize(argument);

                m_tokens.emplace_back();
                m_tokens.back().m_end_arguments = true;
            }
        }

        struct Token
        {
            Tensor m_expr;
            bool m_begin_arguments = false;
            bool m_end_arguments = false;
        };
        std::vector<Token> m_tokens;
    };

    DiscriminationNet::DiscriminationNet()
    {

    }

    void DiscriminationNet::AddPattern(size_t i_pattern_id, 
        const Tensor & i_pattern, const Tensor & i_condition)
    {
        LinearizedExpression pattern(i_pattern);

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
        Tensor m_target;
        std::vector<Substitution> m_substitutions;
    };

    void DiscriminationNet::MatchToken(
        std::vector<Match> & o_matches,
        std::vector<WalkingHead> & io_heads,
        const WalkingHead & i_curr_head, const LinearizedExpression & i_target) const
    {
        const Tensor & token = i_target.GetToken(i_curr_head.m_current_token);

        auto edges = m_edges.equal_range(i_curr_head.m_source_node);
        for(auto it = edges.first; it != edges.second; ++it)
        {
            const Edge & edge = it->second;
            bool matching = false;

            if(IsConstant(edge.m_expr))
                matching = AlwaysEqual(token, edge.m_expr);
            else if(NameIs(edge.m_expr, builtin_names::Identifier))
            {
                matching = Is(token, edge.m_expr);
                if(matching)
                {
                    // skip this sub-expression in the target
                    
                }
            }
            else
            {
                matching = token.GetExpression()->GetName() == edge.m_expr.GetExpression()->GetName();
            }

            if(matching)
            {
                WalkingHead new_head = i_curr_head;
                if(NameIs(edge.m_expr, builtin_names::Identifier))
                {
                    new_head.m_substitutions.emplace_back(Substitution{edge.m_expr, token});
                }
                
                if(edge.m_is_terminal)
                {
                    auto it = m_terminal_states.find(edge.m_dest_node);
                    if(it == m_terminal_states.end())
                        Error("DiscriminationNet: terminal state not found");
                    o_matches.push_back({it->second, i_curr_head.m_substitutions});
                }
                else
                {
                    new_head.m_current_token++;
                    new_head.m_source_node = edge.m_dest_node;
                    io_heads.push_back(std::move(new_head));
                }
            }
        }
    }

    void DiscriminationNet::FindMatches(const Tensor & i_target, std::vector<Match> & o_matches) const
    {
        LinearizedExpression target(i_target);

        std::vector<WalkingHead> heads;
        heads.emplace_back(WalkingHead{0, 0, i_target});

        while(!heads.empty())
        {
            const WalkingHead & curr_head = std::move(heads.back());
            heads.pop_back();

            if(curr_head.m_current_token < target.GetLength())
                MatchToken(o_matches, heads, curr_head, target);
        }
    }
}
