
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "core/diagnostic.h"
#include "core/flags.h"
#include "private/parser.h"
#include "private/lexer.h"
#include "private/alphabet.h"
#include "private/domain.h"
#include "private/expression.h"

namespace djup
{
    // https://en.wikipedia.org/wiki/Operator-precedence_parser

    namespace
    {
        struct Parser
        {

            // tries to parse a type
            static std::optional<Domain> TryParseScalarType(Lexer & i_lexer)
            {
                auto const symbol_id = i_lexer.GetCurrentToken().m_symbol_id;
                if(symbol_id >= SymbolId::FirstType && symbol_id <= SymbolId::LastType)
                    return std::get<Domain>(i_lexer.GetCurrentToken().m_symbol->m_operator_applier);
                else
                    return {};
            }

            static bool ShouldParseDeeper(const Token & i_look_ahead, const Token & i_operator)
            {
                if(!i_look_ahead.IsBinaryOperator())
                    return false;

                if(HasFlag(i_look_ahead.m_symbol->m_flags, SymbolFlags::RightAssociative))
                    return i_look_ahead.m_symbol->m_precedence >= i_operator.m_symbol->m_precedence;
                else
                    return i_look_ahead.m_symbol->m_precedence > i_operator.m_symbol->m_precedence;
            }

            // parses an expression that may be the left-hand-side of a binary operator
            static std::shared_ptr<const Expression> TryParseLeftExpression(Lexer & i_lexer,
                const std::shared_ptr<const Scope> & i_scope)
            {
                /*if(auto token = i_lexer.TryAccept(SymbolId::Literal))
                {
                    // scalar literal
                    if(auto const value = std::get_if<Real>(&token->m_value))
                        return Tensor(*value);
                    else if(auto const value = std::get_if<Integer>(&token->m_value))
                        return Tensor(*value);
                    else if(auto const value = std::get_if<Bool>(&token->m_value))
                        return Tensor(*value);
                    else
                        Error("unrecognized literal type");
                }
                else if(i_lexer.TryAccept(SymbolId::LeftBracket))
                {
                    // stack operator [] - creates a tensor
                    return Stack(ParseExpressionList(i_lexer, i_scope, SymbolId::RightBracket));
                }
                else if (i_lexer.TryAccept(SymbolId::LeftParenthesis))
                {
                    // list operator ()
                    std::vector<Tensor> list = ParseExpressionList(i_lexer, i_scope, SymbolId::RightParenthesis);
                    if(list.size() != 1)
                        Error("expected a list with a sigle element");
                    return list[0];
                }
                else if (i_lexer.TryAccept(SymbolId::LeftBrace))
                {
                    // scope operator {}
                    std::vector<Tensor> list = ParseExpressionList(i_lexer, i_scope->MakeInner(), SymbolId::RightBrace);
                    if(list.size() != 1)
                        Error("expected a list with a sigle element");
                    return list[0];
                }
                else if(i_lexer.TryAccept(SymbolId::If))
                {
                    std::vector<Tensor> operands;

                    do {
                        // condition then value
                        operands.push_back(ParseExpression(i_lexer, i_scope, 0));
                        i_lexer.Accept(SymbolId::Then);
                        operands.push_back(ParseExpression(i_lexer, i_scope, 0));
                    } while(i_lexer.TryAccept(SymbolId::Elif));

                    // else fallback
                    i_lexer.Accept(SymbolId::Else);
                    operands.push_back(ParseExpression(i_lexer, i_scope, 0));

                    return If(operands);
                }
                else if(auto const scalar_type = TryParseScalarType(i_lexer))
                {
                    // variable declaration: both shape and name are optional
                    auto const shape_vector = TryParseExpression(i_lexer, i_scope, 0);
                    auto const name = i_lexer.TryAccept(SymbolId::Name);

                    // if shape_vector is non-null but not a vector, the following will error...
                    TensorType const type = shape_vector ? TensorType{*scalar_type, *shape_vector} : *scalar_type;
                    Tensor variable = MakeVariable(type, name ? name->m_source_chars: "");

                    i_scope->AddVariable(variable);
                    return variable;
                }

                else if (i_lexer.GetCurrentToken().IsUnaryOperator())
                {
                    auto const symbol = i_lexer.GetCurrentToken().m_symbol;

                    i_lexer.Advance();

                    auto const applier = std::get<UnaryApplier>(symbol->m_operator_applier);
                    Tensor const operand = ParseExpression(i_lexer, i_scope, symbol->m_precedence);
                    return applier(operand);
                }

                // context-sensitive unary-to-binary promotion
                else if (i_lexer.TryAccept(SymbolId::BinaryMinus))
                    return -ParseExpression(i_lexer, i_scope, FindSymbol(SymbolId::UnaryMinus).m_precedence);
                else if (i_lexer.TryAccept(SymbolId::BinaryPlus))
                    return ParseExpression(i_lexer, i_scope, FindSymbol(SymbolId::UnaryPlus).m_precedence);

                else if (i_lexer.GetCurrentToken().m_symbol_id == SymbolId::Name)
                {
                    auto const member = i_scope->TryLookup(i_lexer.GetCurrentToken().m_source_chars);
                    if(auto const op_ptr = std::get_if<std::reference_wrapper<const Operator>>(&member))
                    {
                        i_lexer.NextToken();
                        return ParseInvokeOperator(i_lexer, i_scope, op_ptr->get());
                    }
                    else if(auto tensor_ptr = std::get_if<Tensor>(&member))
                    {
                        i_lexer.NextToken();
                        return *tensor_ptr;
                    }
                    else
                        return {};
                }*/

                return {};
            }

            /* given a left operand, tries to parse a binary expression ignoring operators 
                whoose precedence is less than i_min_precedence */
            static std::shared_ptr<const Expression> CombineWithOperator(
                Lexer & i_lexer, const std::shared_ptr<const Scope> & i_scope,
                const std::shared_ptr<const Expression> & i_left_operand, int32_t i_min_precedence)
            {
                std::shared_ptr<const Expression> result = i_left_operand;

                while (i_lexer.GetCurrentToken().IsBinaryOperator()
                    && i_lexer.GetCurrentToken().m_symbol->m_precedence >= i_min_precedence)
                {
                    /* now we have the left-hand operatand and the operator.
                    we just need the right-hand operand. */
                    Token const operator_token = i_lexer.GetCurrentToken();

                    // we have accepted the operator, so we must move to the lext token
                    i_lexer.NextToken();

                    std::shared_ptr<const Expression> right = TryParseLeftExpression(i_lexer, i_scope);
                    if(!right.get())
                        Error("expected right operand for ", GetSymbolChars(operator_token.m_symbol_id));

                    while (ShouldParseDeeper(i_lexer.GetCurrentToken(), operator_token))
                    {
                        right = CombineWithOperator(i_lexer, i_scope,
                            right, i_lexer.GetCurrentToken().m_symbol->m_precedence);
                    }

                    // apply binary operand
                    auto const applier = std::get<BinaryApplier>(operator_token.m_symbol->m_operator_applier);
                    result = applier(result, Tensor(right)).GetExpression();
                }

                return result;
            }

            // parse a complete expression
            static std::shared_ptr<const Expression> ParseExpression(Lexer & i_lexer,
                const std::shared_ptr<const Scope> & i_scope, int32_t i_min_precedence)
            {
                std::shared_ptr<const Expression> const left_operand = TryParseLeftExpression(i_lexer, i_scope);
                if(!left_operand.get())
                    return nullptr;

                // try to combine with a binary operator
                std::shared_ptr<const Expression> result = CombineWithOperator(i_lexer, i_scope, 
                    left_operand, i_min_precedence);

                return result;
            }
        };

    } // namespace

    std::shared_ptr<const Expression> ParseExpression(std::string_view i_source,
        const std::shared_ptr<const Scope> & i_parent_scope)
    {
        Lexer lexer(i_source);
        try
        {
            std::shared_ptr<const Expression> result = Parser::ParseExpression(lexer, i_parent_scope, 0);

            // all the source must be consumed
            if(!lexer.IsSourceOver())
                Error("expected end of source, ", 
                    GetSymbolChars(lexer.GetCurrentToken().m_symbol_id), " found");

            return result;
        }
        catch(const std::exception & i_exc)
        {
            Error(lexer, i_exc.what());
        }
        catch(...)
        {
            Error(lexer, "unspecified error");
        }
    }

} // namespace liquid
