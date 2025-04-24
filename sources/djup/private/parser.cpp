
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <core/diagnostic.h>
#include <core/flags.h>
#include <core/from_chars.h>
#include <private/parser.h>
#include <private/lexer.h>
#include <private/alphabet.h>
#include <djup/expression.h>
#include <private/namespace.h>
#include <private/builtin_names.h>
#include <private/make_expr.h>
#include <djup/tensor.h>

namespace djup
{
    // inspired by https://en.wikipedia.org/wiki/Operator-precedence_parser

    namespace
    {   
        struct ParsingContext
        {
            Lexer & m_lexer;
            Namespace & m_namespace;
            ImmutableVector<char> m_source_file;
        };

        struct ParserImpl
        {
            static std::string AcceptNumericLiteral(std::string_view i_source_chars, int64_t * o_exponent)
            {
                int64_t exponent = 0;
                std::string result;

                size_t i = 0;
                for( ;i < i_source_chars.length() && IsDigit(i_source_chars[i]); i++)
                    result += i_source_chars[i];

                if(i < i_source_chars.length() && i_source_chars[i] == '.')
                {
                    i++;

                    for( ;i < i_source_chars.length() && IsDigit(i_source_chars[i]); i++)
                    {
                        result += i_source_chars[i];
                        exponent--;
                    }
                }

                if(i < i_source_chars.length() && (i_source_chars[i] == 'e' || i_source_chars[i] == 'E'))
                {
                    i++;

                    if(i_source_chars[i] == '+')
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
            static std::vector<Tensor> ParseExpressionList(
                ParsingContext & i_context, SymbolId i_terminator_symbol)
            {
                Lexer & lexer = i_context.m_lexer;
                
                std::vector<Tensor> result;
                while(!lexer.TryAccept(i_terminator_symbol))
                {
                    result.push_back(ParseExpression(i_context));
                    lexer.TryAccept(SymbolId::Comma);
                }
                return result;
            }

            static Tensor ParseIdentifier(Name i_name, ParsingContext & i_context)
            {
                Lexer & lexer = i_context.m_lexer;

                Tensor shape;
                TensorType type;
                Name name;

                if (i_context.m_namespace.IsScalarType(i_name))
                {
                    // parse shape                
                    if (lexer.TryAccept(SymbolId::LeftBracket))
                        shape = Stack(ParseExpressionList(i_context, SymbolId::RightBracket));

                    // re-parse name
                    if (std::optional<Token> name_token = lexer.TryAccept(SymbolId::Name))
                        name = name_token->m_source_chars;

                    // make type
                    type = TensorType(i_name, std::move(shape));
                }
                else
                {
                    name = std::move(i_name);
                }

                ExpressionMetadata metadata;

                // arguments or parameters
                std::vector<Tensor> arguments;
                if (lexer.TryAccept(SymbolId::LeftParenthesis))
                    arguments = ParseExpressionList(i_context, SymbolId::RightParenthesis);
                else
                    metadata.m_is_identifier = true;

                Tensor result = MakeExpression(type,
                    std::move(name), arguments, std::move(metadata));

                return result;
            }

            // parses an expression that may be the left-hand-side of a binary operator
            static Tensor ParseLeftExpression(ParsingContext & i_context)
            {             
                Lexer & lexer = i_context.m_lexer;

                if (std::optional<Token> name_token = lexer.TryAccept(SymbolId::Name))
                {
                    Name name = name_token->m_source_chars;
                    return ParseIdentifier(std::move(name), i_context);
                }
                else if(std::optional<Token> token = lexer.TryAccept(SymbolId::NumericLiteral))
                {
                    int64_t exponent = 0;
                    std::string value_chars = AcceptNumericLiteral(token->m_source_chars, &exponent);

                    // to do: use multi-precision ints
                    if(exponent == 0)
                        return Tensor(Parse<int64_t>(value_chars));
                    else
                        return Tensor(Parse<int64_t>(value_chars)) * Pow(Tensor(10), Tensor(exponent));
                }
                else if(std::optional<Token> token = lexer.TryAccept(SymbolId::BoolLiteral))
                {
                    if(token->m_source_chars == "true")
                        return MakeLiteral<true>();
                    else if(token->m_source_chars == "false")
                        return MakeLiteral<false>();
                    else
                        Error("Unrecognized bool literal: ", token->m_source_chars);
                }
                else if(lexer.TryAccept(SymbolId::LeftBracket))
                {
                    // stack operators [] - create a tensor
                    return Stack(ParseExpressionList(i_context, SymbolId::RightBracket));
                }
                else if(lexer.TryAccept(SymbolId::LeftParenthesis))
                {
                    std::vector<Tensor> arguments = ParseExpressionList(i_context, SymbolId::RightParenthesis);
                    if(arguments.size() == 1)
                        return std::move(arguments.front());
                    else
                        return Tuple(arguments);
                }
                else if(lexer.TryAccept(SymbolId::LeftBrace))
                {
                    // namespace operator {}
                    return ParseNamespace(i_context, SymbolId::RightBrace);
                }
                else if(lexer.TryAccept(SymbolId::If))
                {
                    std::vector<Tensor> operands;

                    do {
                        // condition then value
                        operands.push_back(ParseExpression(i_context));
                        lexer.TryAccept(SymbolId::Then);
                        operands.push_back(ParseExpression(i_context));
                    } while(lexer.TryAccept(SymbolId::Elif));

                    // else fall-back
                    lexer.Accept(SymbolId::Else);
                    operands.push_back(ParseExpression(i_context));

                    return If(operands);
                }
                else if (lexer.GetCurrentToken().IsUnaryOperator())
                {
                    const Symbol * symbol = lexer.GetCurrentToken().m_symbol;

                    lexer.NextToken();

                    auto const applier = std::get<UnaryApplier>(symbol->m_operator_applier);
                    const Tensor operand = ParseExpression(i_context, symbol->m_precedence);
                    return applier(operand);
                }

                /* context-sensitive unary-to-binary promotion: binary operator occurrences (respecting 
                    the white symmetry rule) are promoted to unary operators if there is no left operand. */
                else if (lexer.TryAccept(SymbolId::BinaryMinus))
                    return -ParseExpression(i_context, FindSymbol(SymbolId::UnaryMinus).m_precedence);
                else if (lexer.TryAccept(SymbolId::BinaryPlus))
                    return ParseExpression(i_context, FindSymbol(SymbolId::UnaryPlus).m_precedence);
                
                // return
                else if (lexer.TryAccept(SymbolId::Return))
                {
                    Tensor value = ParseExpression(i_context);
                    return MakeExpression({}, builtin_names::Return, 
                        { std::move(value) }, {});
                }

                else
                    Error("Expected expression");
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
                whose precedence is less than i_min_precedence. The operator cannot be 
                preceded by a line_break. */
            static Tensor CombineWithOperator(ParsingContext & i_context,
                const Tensor & i_left_operand, int32_t i_min_precedence)
            {
                Lexer & lexer = i_context.m_lexer;

                Tensor result = i_left_operand;

                while (!lexer.GetCurrentToken().m_follows_line_break
                    && lexer.GetCurrentToken().IsBinaryOperator()
                    && lexer.GetCurrentToken().m_symbol->m_precedence >= i_min_precedence)
                {
                    /* now we have the left-hand operand and the operator.
                       we just need the right-hand operand. */
                    Token const operator_token = lexer.GetCurrentToken();

                    // we have accepted the operator, so we must move to the next token
                    lexer.NextToken();

                    Tensor right = ParseLeftExpression(i_context);
                    if(IsEmpty(right))
                        Error("expected right operand for ", GetSymbolChars(operator_token.m_symbol_id));

                    while (ShouldParseDeeper(lexer.GetCurrentToken(), operator_token))
                    {
                        right = CombineWithOperator(i_context,
                            right, lexer.GetCurrentToken().m_symbol->m_precedence);
                    }

                    // apply binary operand
                    auto const applier = std::get<BinaryApplier>(operator_token.m_symbol->m_operator_applier);
                    result = applier(result, right);
                }

                return result;
            }

            // parse a complete expression
            static Tensor ParseExpression(ParsingContext & i_context, int32_t i_min_precedence = 0)
            {
                Tensor result = ParseLeftExpression(i_context);

                // try to combine with a binary operator
                result = CombineWithOperator(i_context, result, i_min_precedence);

                // repetition operators
                Tensor (*repetition)(Span<Tensor const>) = nullptr;
                if(i_context.m_lexer.TryAcceptInline(SymbolId::RepetitionsZeroToOne))
                    repetition = &RepetitionsZeroToOne;
                else if(i_context.m_lexer.TryAcceptInline(SymbolId::RepetitionsOneToMany))
                    repetition = &RepetitionsOneToMany;
                else if(i_context.m_lexer.TryAcceptInline(SymbolId::RepetitionsZeroToMany))
                    repetition = &RepetitionsZeroToMany;
                if(repetition != nullptr)
                {
                    // a tuple as argument of a repetition decays to its arguments
                    if(NameIs(result, builtin_names::Tuple))
                        return (*repetition)(result.GetExpression()->GetArguments());
                    else
                        return (*repetition)({result});
                }

                return result;
            }

            static Tensor ParseStatement(ParsingContext & i_context)
            {
                Tensor expression = ParseExpression(i_context, 0);

                /* if the expression has a name, and it's not followed by line break, it may 
                   be a substitution axiom */
                if(!expression.GetExpression()->GetName().IsEmpty())
                {
                    Tensor when;
                    if(i_context.m_lexer.TryAccept(SymbolId::When))
                        when = ParseExpression(i_context);

                    if(i_context.m_lexer.TryAccept(SymbolId::SubstitutionAxiom))
                    {
                        Tensor right_hand_side = ParseExpression(i_context);
                        expression = MakeExpression({}, builtin_names::SubstitutionAxiom,
                            { expression, right_hand_side, when }, {});
                    }
                    else if (i_context.m_lexer.TryAccept(SymbolId::LeftBrace))
                    {
                        Tensor right_hand_side = ParseNamespace(i_context, SymbolId::RightBrace);
                        expression = MakeExpression({}, builtin_names::SubstitutionAxiom,
                            { expression, right_hand_side, when }, {});
                    }
                    else if(!IsEmpty(when))
                        Error("'when' clause without axiom");
                }

                return expression;
            }

            /** Parses a namespace, assuming that an opening brace has already
                been parsed. A return statement is expected as last statement. */
            static Tensor ParseNamespace(ParsingContext& i_context,
                SymbolId i_terminal_symbol)
            {
                Lexer& lexer = i_context.m_lexer;

                std::vector<Tensor> statements;
                while (!lexer.TryAccept(i_terminal_symbol))
                {
                    statements.push_back(ParseStatement(i_context));
                }
                return MakeNamespace(statements);
            }

        }; // struct ParserImpl

    } // namespace

    Tensor ParseExpression(std::string_view i_source)
    {
        Lexer lexer(i_source);
        if(lexer.IsSourceOver())
            return {};

        try
        {
            Namespace temporary_namespace("", GetDefaultNamespace());
            ParsingContext context{lexer, temporary_namespace };
            Tensor result = ParserImpl::ParseExpression(context);

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
        catch(const StaticCStrException & i_exc)
        {
            Error(lexer, i_exc.c_str());
        }
        catch(...)
        {
            Error(lexer, "unspecified error");
        }
    }

    Tensor ParseNamespace(std::string_view i_source)
    {
        Lexer lexer(i_source);
        if (lexer.IsSourceOver())
            return {};

        try
        {
            Namespace temporary_namespace("", GetDefaultNamespace());
            ParsingContext context{ lexer, temporary_namespace };
            Tensor result = ParserImpl::ParseNamespace(context, SymbolId::EndOfSource);

            // all the source must be consumed
            if (!lexer.IsSourceOver())
                Error("expected end of source, ",
                    GetSymbolChars(lexer.GetCurrentToken().m_symbol_id), " found");

            return result;
        }
        catch (const std::exception& i_exc)
        {
            Error(lexer, i_exc.what());
        }
        catch (const StaticCStrException& i_exc)
        {
            Error(lexer, i_exc.c_str());
        }
        catch (...)
        {
            Error(lexer, "unspecified error");
        }
    }

} // namespace djup
