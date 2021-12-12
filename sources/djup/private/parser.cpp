
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <core/diagnostic.h>
#include <core/flags.h>
#include <core/from_chars.h>
#include <private/parser.h>
#include <private/lexer.h>
#include <private/alphabet.h>
#include <private/domain.h>
#include <private/expression.h>

namespace djup
{
    // https://en.wikipedia.org/wiki/Operator-precedence_parser

    namespace
    {
        struct ParserImpl
        {
            static std::string ParseNumericLiteral(std::string_view i_source_chars,
                int64_t * o_exponent)
            {
                int64_t exponent = 0;
                std::string result;
                
                size_t i = 0;
                for( ;i < i_source_chars.length() && IsDigit(i_source_chars[i]); i++)
                    result += i_source_chars[i];

                if(i_source_chars[i] == '.')
                {
                    i++;

                    for( ;i < i_source_chars.length() && IsDigit(i_source_chars[i]); i++)
                    {
                        result += i_source_chars[i];
                        exponent--;
                    }
                }

                if(i_source_chars[i] == 'e' || i_source_chars[i] == 'E')
                {
                    i++;

                    int64_t explicit_exp = Parse<int64_t>(i_source_chars.substr(i));
                    exponent += explicit_exp;
                }
                else if(i < i_source_chars.length())
                {
                    Error("Unrecognized content in numeric literal: ", i_source_chars);
                }

                *o_exponent = exponent;
                return result;
            }

            // parses a space-separated or comma-separated list of expressions
            static std::vector<Tensor> ParseExpressionList(Lexer & i_lexer, 
                const std::shared_ptr<const Scope> & i_scope, SymbolId i_terminator_symbol)
            {
                std::vector<Tensor> result;
                while(!i_lexer.TryAccept(i_terminator_symbol))
                {
                    result.push_back(ParseExpression(i_lexer, i_scope, 0));
                    i_lexer.TryAccept(SymbolId::Comma);
                }
                return result;
            }

            // parses an expression that may be the left-hand-side of a binary operator
            static std::optional<Tensor> ParseLeftExpression(Lexer & i_lexer,
                const std::shared_ptr<const Scope> & i_scope)
            {
                if(i_lexer.GetCurrentToken().m_symbol_id >= SymbolId::FirstScalarType &&
                    i_lexer.GetCurrentToken().m_symbol_id <= SymbolId::LastScalarType)
                {
                    // found scalar type
                    if(i_lexer.TryAccept(SymbolId::LeftBracket))
                    {
                        // parse the shape
                        std::vector<Tensor> shape = ParseExpressionList(i_lexer, i_scope, SymbolId::RightBracket);
                    }
                }
                if(std::optional<Token> token = i_lexer.TryAccept(SymbolId::NumericLiteral))
                {
                    int64_t exponent = 0;
                    std::string value_chars = ParseNumericLiteral(token->m_source_chars, &exponent);
                    
                    // to do: use multi-precion ints
                    return Tensor(Parse<int64_t>(value_chars)) / Pow(Tensor(10), Tensor(exponent));
                }
                else if(std::optional<Token> token = i_lexer.TryAccept(SymbolId::BoolLiteral))
                {
                    if(token->m_source_chars == "true")
                        return true;
                    else if(token->m_source_chars == "false")
                        return false;
                    else
                        Error("Unrecognized bool literal: ", token->m_source_chars);
                }
                else if(i_lexer.TryAccept(SymbolId::LeftBracket))
                {
                    // stack operator [] - creates a tensor
                    return Stack(ParseExpressionList(i_lexer, i_scope, SymbolId::RightBracket));
                }
                else if(i_lexer.TryAccept(SymbolId::LeftParenthesis))
                {
                    return ParseExpression(i_lexer, i_scope, 0);
                }
                /*else if(i_lexer.TryAccept(SymbolId::LeftBrace))
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
                        operands.push_back(TryParseExpression(i_lexer, i_scope, 0));
                        i_lexer.Accept(SymbolId::Then);
                        operands.push_back(TryParseExpression(i_lexer, i_scope, 0));
                    } while(i_lexer.TryAccept(SymbolId::Elif));

                    // else fallback
                    i_lexer.Accept(SymbolId::Else);
                    operands.push_back(TryParseExpression(i_lexer, i_scope, 0));

                    return If(operands);
                }
                else if(auto const scalar_type = TryParseScalarDomain(i_lexer))
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
                    Tensor const operand = TryParseExpression(i_lexer, i_scope, symbol->m_precedence);
                    return applier(operand);
                }

                // context-sensitive unary-to-binary promotion
                else if (i_lexer.TryAccept(SymbolId::BinaryMinus))
                    return -TryParseExpression(i_lexer, i_scope, FindSymbol(SymbolId::UnaryMinus).m_precedence);
                else if (i_lexer.TryAccept(SymbolId::BinaryPlus))
                    return TryParseExpression(i_lexer, i_scope, FindSymbol(SymbolId::UnaryPlus).m_precedence);

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

            // tries to parse a domain
            static std::optional<Domain> TryParseScalarDomain(Lexer & i_lexer)
            {
                auto const symbol_id = i_lexer.GetCurrentToken().m_symbol_id;
                if(symbol_id >= SymbolId::FirstScalarType && symbol_id <= SymbolId::LastScalarType)
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

            /* given a left operand, tries to parse a binary expression ignoring operators 
                whoose precedence is less than i_min_precedence */
            static Tensor CombineWithOperator(
                Lexer & i_lexer, const std::shared_ptr<const Scope> & i_scope,
                const Tensor & i_left_operand, int32_t i_min_precedence)
            {
                Tensor result = i_left_operand;

                while (i_lexer.GetCurrentToken().IsBinaryOperator()
                    && i_lexer.GetCurrentToken().m_symbol->m_precedence >= i_min_precedence)
                {
                    /* now we have the left-hand operatand and the operator.
                    we just need the right-hand operand. */
                    Token const operator_token = i_lexer.GetCurrentToken();

                    // we have accepted the operator, so we must move to the lext token
                    i_lexer.NextToken();

                    std::optional<Tensor> right = ParseLeftExpression(i_lexer, i_scope);
                    if(!right)
                        Error("expected right operand for ", GetSymbolChars(operator_token.m_symbol_id));

                    while (ShouldParseDeeper(i_lexer.GetCurrentToken(), operator_token))
                    {
                        *right = CombineWithOperator(i_lexer, i_scope,
                            *right, i_lexer.GetCurrentToken().m_symbol->m_precedence);
                    }

                    // apply binary operand
                    auto const applier = std::get<BinaryApplier>(operator_token.m_symbol->m_operator_applier);
                    result = applier(result, *right);
                }

                return result;
            }

            // parse a complete expression
            static std::optional<Tensor> TryParseExpression(Lexer & i_lexer,
                const std::shared_ptr<const Scope> & i_scope, int32_t i_min_precedence)
            {
                if(std::optional<Tensor> const left_operand = ParseLeftExpression(i_lexer, i_scope))
                {
                    // try to combine with a binary operator
                    Tensor result = CombineWithOperator(i_lexer, i_scope, 
                        *left_operand, i_min_precedence);
                    return result;
                }
                else
                    return {};
            }

            static Tensor ParseExpression(Lexer & i_lexer, const std::shared_ptr<const Scope> & i_scope, int32_t i_min_precedence)
            {
                if(auto expression = TryParseExpression(i_lexer, i_scope, i_min_precedence))
                    return *expression;
                else
                    Error("expected an expression");
            }
        };

    } // namespace

    Tensor TryParseExpression(std::string_view i_source,
        const std::shared_ptr<const Scope> & i_parent_scope)
    {
        Lexer lexer(i_source);
        try
        {
            Tensor result = ParserImpl::ParseExpression(lexer, i_parent_scope, 0);

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
