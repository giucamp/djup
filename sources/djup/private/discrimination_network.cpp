
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/discrimination_network.h>
#include <private/substitute_by_predicate.h>
#include <vector>

namespace djup
{
    /** Immutable flat representation of an expression.
        An expression is a directed acyclic graph. The construction of a flat representation 
        adds an overhead cost, but it greatly simplifies the pattern matching algorithms. */
    class DiscriminationNetwork::LinearizedTarget
    {
    public:

        LinearizedTarget(const Tensor & i_target)
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
            if(!i_target.GetExpression()->GetArguments().empty())
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
        AddSubPattern(0, i_pattern_id, i_pattern, i_condition);
    }

    size_t DiscriminationNetwork::AddSubPattern(size_t i_source_node, size_t i_pattern_id, 
        const Tensor & i_pattern, const Tensor & i_condition)
    {
        size_t dest_node = m_next_node_index++;

        /*if(IsConstant(i_pattern))
        {
            // constant
            Edge edge{dest_node, EdgeKind::Constant, i_pattern };
            m_edges.insert(std::make_pair(i_source_node, std::move(edge)));
        }
        else if(IsVariable(i_pattern))
        {
            // variable
            Edge edge{dest_node, EdgeKind::Variable, i_pattern };
            m_edges.insert(std::make_pair(i_source_node, std::move(edge)));
        }
        else
        {
            // name
            Edge edge{dest_node, EdgeKind::Name, i_pattern };
            m_edges.insert(std::make_pair(i_source_node, std::move(edge)));

            for(const Tensor & argument : i_pattern.GetExpression()->GetArguments())
            {
                dest_node = AddSubPattern(dest_node, i_pattern_id, argument, i_condition);
            }
        }*/

        return dest_node;
    }

    struct DiscriminationNetwork::WalkingHead
    {
        size_t m_source_node = 0;
        size_t m_current_token = 0;
        Tensor m_target;
    };

    bool DiscriminationNetwork::MatchToken(WalkingHead & i_head, const LinearizedTarget & i_target) const
    {
        const Expression & token = i_target.GetToken(i_head.m_current_token);

        auto edges = m_edges.equal_range(i_head.m_source_node);
        for(auto it = edges.first; it != edges.second; ++it)
        {
            const Edge & edge = it->second;
            switch(edge.m_kind)
            {
            case EdgeKind::Constant:
                if(!AlwaysEqual(edge.m_expr, token))
                    return false;

                i_head.m_current_token++;
                i_head.m_source_node = edge.m_dest_node;
                return true;

            case EdgeKind::Variable:
                
                break;

            case EdgeKind::Name:                
                if(edge.m_expr.GetName() != token.GetName())
                    return false;

                i_head.m_current_token++;
                i_head.m_source_node = edge.m_dest_node;
                return true;

            default:
                Error("DiscriminationNetwork: unrecognized edje kind");
            }
        }

        return false;
    }

    void DiscriminationNetwork::FindMatches(const Tensor & i_target, std::vector<Match> o_matches) const
    {
        LinearizedTarget target(i_target);

        std::vector<WalkingHead> heads;
        heads.emplace_back(WalkingHead{0, 0, i_target});

        while(!heads.empty())
        {
            const WalkingHead & head = heads.back();

            // if(MatchToken())
            {

            }

            heads.pop_back();
        }

        /*if(IsConstant(i_target))
        {
            // constant
            Edge edge{i_node_index, dest_node, EdgeKind::Constant, i_pattern };
            m_edges.insert(std::make_pair(edge.GetHash(), std::move(edge)));
        }
        else if(IsVariable(i_target))
        {
            // variable
            Edge edge{i_node_index, dest_node, EdgeKind::Variable, i_pattern };
            m_edges.insert(std::make_pair(edge.GetHash(), std::move(edge)));
        }
        else
        {
            // name
            Edge edge{i_node_index, dest_node, EdgeKind::Name, i_pattern };
            m_edges.insert(std::make_pair(edge.GetHash(), std::move(edge)));
        }*/
    }
}
