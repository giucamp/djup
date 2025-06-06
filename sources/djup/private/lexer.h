
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <private/common.h>
#include <private/alphabet.h>
#include <string_view>
#include <optional>

namespace djup
{
    /* A token is a range of chars recognized by the lexer. Accessing the content of this struct
        outside the lifetime of the Lexer that returned it causes undefined behavior. */
    struct Token
    {
        SymbolId m_symbol_id = SymbolId::EndOfSource; /* describes the category of the token (see SymbolId). */ 
     
        bool m_follows_line_break = false; /** weather the spaces before this token include any '/n' */

        const Symbol * m_symbol = nullptr; /** if non-null contains detail about char-representation,
            precedence, associativity and applier of the token. This pointer is null for tokens with no 
            fixed char-representation, like literals and names, and also for SymbolId::EndOfSource. */

        std::string_view m_source_chars; /** the portion of source that composes the token, excluding 
            any heading or tailing space. */

        Token() = default;

        Token(const Symbol & i_symbol)
            : m_symbol_id(i_symbol.m_id), m_symbol(&i_symbol) {}

        Token(SymbolId i_symbol_id)
            : m_symbol_id(i_symbol_id) {}

        bool IsUnaryOperator() const
            { return m_symbol != nullptr && m_symbol->IsUnaryOperator(); }

        bool IsBinaryOperator() const
            { return m_symbol != nullptr && m_symbol->IsBinaryOperator(); }
    };

    /** Visits linearly the tokens of a djup textual source. Used by the parser. */
    class Lexer
    {
    public:

        Lexer(const Lexer &) = delete;
        Lexer & operator = (const Lexer &) = delete;

        /** Constructs a Lexer to tokenize the specified source. If the memory referred by
            this string_view is not valid through the entire life-time of the Lexer, the 
            behavior is undefined. */
        Lexer(std::string_view i_source);

        /** Returns the current token without accepting it. Heading spaces are skipped */
        const Token & GetCurrentToken() const { return m_curr_token; }

        /** If after skipping spaces the current token matches the provided symbol id,
            accepts and returns it. Otherwise the return value is empty. The heading 
            spaces can contain line-breaks. */
        std::optional<Token> TryAccept(SymbolId i_symbol_id);

        /** If after skipping spaces the current token matches the provided symbol id,
            accepts and returns it. Otherwise an exception is thrown. The heading spaces 
            can contain line-breaks. */
        Token Accept(SymbolId i_symbol_id);

        /** If after skipping spaces the current token matches the provided symbol id,
            accepts and returns it. The heading spaces cannot contain line-breaks. */
        std::optional<Token> TryAcceptInline(SymbolId i_symbol_id);

        /** If after skipping spaces the current token matches the provided symbol id,
            accepts and returns it. The heading spaces cannot contain line-breaks. */
        Token AcceptInline(SymbolId i_symbol_id);

        /** Moves to the next token, and returns it. Heading spaces are ignored. 
            The behavior is undefined if the id of the current symbol is 
            SymbolId::EndOfSource (or equivalently IsSourceOver() returns true). */
        const Token & NextToken();

        /** Equivalent to GetCurrentToken().m_symbol_id == SymbolId::EndOfSource */
        bool IsSourceOver() const;

        /** Returns the all source, as passed to the constructor. */
        std::string_view GetWholeSource() const { return m_whole_source; }

    private:
        void NextTokenImpl();

    private:
        Token m_curr_token;
        std::string_view m_remaining_source;
        std::string_view m_whole_source;
    };

    /** Returns true is the argument is a white char, like a space or a tabulation */
    bool IsSpace(uint32_t i_char);
    
    /** Returns true is the argument is a numeric digit */
    bool IsDigit(uint32_t i_char);

    /** Returns true is the argument is a letter, underscore or upperscore.
        For simplicity all chars from 0x7F (DEL) on are also considered letters. */
    bool IsAlpha(uint32_t i_char);

    /** IsAlpha(i_char) || i_char == '_' || IsDigit(i_char) */
    bool IsAlphaOrUnderscore(uint32_t i_char);

} // namespace djup

namespace core
{
    /** Converts to char the current state of a Lexer. The result includes
        the line number and a '^' char in the next line under the current
        consume location of the lexer. */
    template <> struct CharWriter<djup::Lexer>
    {
        void operator() (CharBufferView& i_dest, const djup::Lexer& i_source) noexcept;
    };
}