
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <private/common.h>
#include <core/diagnostic.h>
#include <core/flags.h>
#include <djup/tensor.h>

namespace djup
{
    /* Id of a symbol recognizable by the Lexer. Usually symbols in the alphabet have their
        own SymbolId, anyway some SymbolId's (like dynamic symbols and special symbols)
        have no matching alphabet symbol. */
    enum class SymbolId : uint32_t
    {
        // arithmetic unary operators
        UnaryPlus, UnaryMinus,

        // arithmetic binary operators
        BinaryPlus, BinaryMinus, Mul, Div, Pow,

        // comparison
        Equal, NotEqual, Less, Greater, LessOrEqual, GreaterOrEqual,

        // boolean operators
        And, Or, Not,

        // bracket
        LeftParenthesis,    RightParenthesis, 
        LeftBracket,        RightBracket, 
        LeftBrace,          RightBrace,
        
        // pattern matching
        SubstitutionAxiom,
        When,
        RepetitionsOneToMany,
        RepetitionsZeroToMany,
        RepetitionsZeroToOne,

        // if
        If, Then, Elif, Else,

        // misc
        Comma, Of, Is,

        // dynamic symbols
        Name, NumericLiteral, BoolLiteral,

        // special symbols
        EndOfSource,
    };

    enum class SymbolFlags : uint32_t
    {
        None = 0,
        RightAssociative = (1 << 0) // binary operator associable from left to right
    };

    constexpr SymbolFlags operator | (SymbolFlags i_first, SymbolFlags i_second)
        { return CombineFlags(i_first, i_second); }

    /** Type for a function called to associate an operator to an applier function.
        For example "!a" becomes "Not(a)". */
    using UnaryApplier = Tensor(*)(const Tensor& i_operand);

    /** Type for a function called to associate an operator to an applier function. 
        For example "a * b" becomes "Mul(a b)", and  "a == b" becomes "Equal(a b)." */
    using BinaryApplier = Tensor (*)(const Tensor & i_left, const Tensor & i_right);

    using OperatorApplier = std::variant<std::monostate, UnaryApplier, BinaryApplier>;

    // element of the alphabet
    struct Symbol
    {
        std::string_view m_chars;
        SymbolId m_id = SymbolId::EndOfSource;
        OperatorApplier m_operator_applier;
        int32_t m_precedence = -1; // precedence honored by the parser
        SymbolFlags m_flags = SymbolFlags::None;

        constexpr bool IsUnaryOperator() const
            { return std::holds_alternative<UnaryApplier>(m_operator_applier); }

        constexpr bool IsBinaryOperator() const
            { return std::holds_alternative<BinaryApplier>(m_operator_applier); }
    };

    #ifdef __GNUC__
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    #endif

    /* The lexer recognizes symbols in the order they appear in this array.
        Shadowing must be considered: so ">" must appear after ">=", otherwise
        the former would always shadows the latter. */
    constexpr Symbol g_alphabet[] = 
    {
        // comparison
        { "==",         SymbolId::Equal,             Equal,           100 },
        { "!=",         SymbolId::NotEqual,          NotEqual,        100 },
        { ">=",         SymbolId::GreaterOrEqual,    operator >=,     100 },
        { "<=",         SymbolId::LessOrEqual,       operator <=,     100 },
        { ">",          SymbolId::Greater,           operator >,      100 },
        { "<",          SymbolId::Less,              operator <,      100 },

        // boolean operators
        { "or",         SymbolId::Or,                operator ||,     200 },
        { "and",        SymbolId::And,               operator &&,     300 },
        { "not",        SymbolId::Not,               operator !,      300 },

        // arithmetic binary operators
        { "+",          SymbolId::BinaryPlus,        (BinaryApplier)operator +,     500 },
        { "-",          SymbolId::BinaryMinus,       (BinaryApplier)operator -,     500 },
        { "*",          SymbolId::Mul,               operator *,                    600 },
        { "/",          SymbolId::Div,               operator /,                    600 },
        { "^",          SymbolId::Pow,               Pow,                           600, SymbolFlags::RightAssociative },

        // unary arithmetic operators
        { "+",          SymbolId::UnaryPlus,         (UnaryApplier)operator +,      700 },
        { "-",          SymbolId::UnaryMinus,        (UnaryApplier)operator -,      700 },

        // brackets
        { "(",          SymbolId::LeftParenthesis                                       },
        { ")",          SymbolId::RightParenthesis                                      },
        { "[",          SymbolId::LeftBracket                                           },
        { "]",          SymbolId::RightBracket                                          },
        { "{",          SymbolId::LeftBrace                                             },
        { "}",          SymbolId::RightBrace                                            },

        // pattern matching
        { "=",          SymbolId::SubstitutionAxiom                                     },
        { "when",       SymbolId::When                                                  },
        { "...",        SymbolId::RepetitionsZeroToMany                                 },
        { "..",         SymbolId::RepetitionsOneToMany                                  },
        { "?",          SymbolId::RepetitionsZeroToOne                                  },

        // if
        { "if",         SymbolId::If                                                    },
        { "then",       SymbolId::Then                                                  },
        { "elif",       SymbolId::Elif                                                  },
        { "else",       SymbolId::Else                                                  },

        // misc
        { ",",          SymbolId::Comma                                                 },
        { "of ",        SymbolId::Of                                                    },
        { "is",         SymbolId::Is,                BinaryApplier{},               400 },
    };

    #ifdef __GNUC__
        #pragma GCC diagnostic pop
    #endif

    constexpr const Symbol * TryFindSymbol(SymbolId i_symbol_id)
    {
        for(const Symbol & symbol : g_alphabet)
            if(symbol.m_id == i_symbol_id)
                return &symbol;

        return nullptr;
    }

    constexpr const Symbol & FindSymbol(SymbolId i_symbol_id)
    {
        if(auto symbol = TryFindSymbol(i_symbol_id))
            return *symbol;

        Error("FindSymbol - unrecognized symbol id: ", static_cast<int>(i_symbol_id));
    }

    constexpr std::string_view GetSymbolChars(SymbolId i_symbol_id)
    {
        if(auto symbol = TryFindSymbol(i_symbol_id))
            return symbol->m_chars;

        switch(i_symbol_id)
        {
        case SymbolId::Name:
            return "<name>";

        case SymbolId::NumericLiteral:
            return "<numeric_literal>";

        case SymbolId::BoolLiteral:
            return "<bool_literal>";

        case SymbolId::EndOfSource:
            return "<end_of_source>";

        default:
            Error("GetSymbolChars - unrecognized symbol id: ", static_cast<int>(i_symbol_id));
        }

    }

} // namespace djup
