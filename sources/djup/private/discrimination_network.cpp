
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/discrimination_network.h>
#include <vector>

namespace djup
{
    /** Immutable flat representation of an expression.
        An expression is a directed acyclic graph. The construction of a flat representation 
        adds an overhead cost, but it greatly simplifies the pattern matching algorithms. */
    class DiscriminationNetwork::LinearizedExpression
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

        const Expression & GetToken(size_t i_index) const
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
            m_tokens.push_back(Token{*i_target.GetExpression()});

            // a constant is added as a single token, even if it's composite
            if(!IsConstant(i_target) && !i_target.GetExpression()->GetArguments().empty())
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
            Expression m_expr;
            bool m_begin_arguments = false;
            bool m_end_arguments = false;
        };
        std::vector<Token> m_tokens;
    };

    DiscriminationNetwork::DiscriminationNetwork()
    {

    }

    void DiscriminationNetwork::AddPattern(size_t i_pattern_id, 
        const Tensor & i_pattern, const Tensor & i_condition)
    {
        LinearizedExpression pattern(i_pattern);

        const size_t pattern_length = pattern.GetLength();
        size_t prev_node = 0;

        for(size_t token_index = 0; token_index < pattern_length; token_index++)
        {
            const Expression & token = pattern.GetToken(token_index);

            Edge edge;
            if(IsConstant(token))
                edge.m_kind = EdgeKind::Constant;
            else if(IsVariable(token))
                edge.m_kind = EdgeKind::Variable;
            else
                edge.m_kind = EdgeKind::Name;

            edge.m_begin_arguments = pattern.BeginsArguments(token_index);
            edge.m_end_arguments = pattern.EndsArguments(token_index);
            edge.m_expr = token;

            size_t dest_node = m_next_node_index++;
            edge.m_dest_node = dest_node;

            m_edges.insert(std::make_pair(prev_node, std::move(edge)));
            prev_node = dest_node;
        }
    }

    struct DiscriminationNetwork::WalkingHead
    {
        size_t m_source_node = 0;
        size_t m_current_token = 0;
        Tensor m_target;
        std::vector<Substitution> m_substitutions;
    };

    void DiscriminationNetwork::MatchToken(
        std::vector<Match> & o_matches,
        std::vector<WalkingHead> & io_heads,
        const WalkingHead & i_curr_head, const LinearizedExpression & i_target) const
    {
        const Expression & token = i_target.GetToken(i_curr_head.m_current_token);

        auto edges = m_edges.equal_range(i_curr_head.m_source_node);
        for(auto it = edges.first; it != edges.second; ++it)
        {
            const Edge & edge = it->second;
            switch(edge.m_kind)
            {
            case EdgeKind::Constant:
                if(AlwaysEqual(edge.m_expr, token))
                {
                    WalkingHead new_head = i_curr_head;
                    new_head.m_current_token++;
                    new_head.m_source_node = edge.m_dest_node;
                    io_heads.push_back(std::move(new_head));
                }
                break;

            case EdgeKind::Name:
                if(edge.m_expr.GetName() == token.GetName())
                {
                    WalkingHead new_head = i_curr_head;
                    new_head.m_current_token++;
                    new_head.m_source_node = edge.m_dest_node;
                    io_heads.push_back(std::move(new_head));
                }
                break;

            case EdgeKind::Variable:
                if(TypeMatches(token.GetType(), edge.m_expr.GetType()))
                {
                    WalkingHead new_head = i_curr_head;
                    new_head.m_current_token++;
                    new_head.m_source_node = edge.m_dest_node;
                    new_head.m_substitutions.emplace_back(Substitution{edge.m_expr, token});
                    io_heads.push_back(std::move(new_head));
                }
                break;

            case EdgeKind::Terminal:
            {
                Match match;
                match.m_substitutions = i_curr_head.m_substitutions;
                match.m_pattern_id = edge.m_pattern_id;
                o_matches.push_back(std::move(match));
                break;
            }

            default:
                Error("DiscriminationNetwork: unrecognized edje kind");
            }
        }
    }

    void DiscriminationNetwork::FindMatches(const Tensor & i_target, std::vector<Match> & o_matches) const
    {
        LinearizedExpression target(i_target);

        std::vector<WalkingHead> heads;
        heads.emplace_back(WalkingHead{0, 0, i_target});

        while(!heads.empty())
        {
            const WalkingHead & curr_head = std::move(heads.back());
            heads.pop_back();

            MatchToken(o_matches, heads, curr_head, target);
        }
    }
}
