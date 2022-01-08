
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <private/alphabet.h>
#include <string_view>
#include <optional>

namespace djup
{
    // a token is a range of chars recognized by the lexer
    struct Token
    {
        SymbolId m_symbol_id = SymbolId::EndOfSource;
        bool m_follows_line_break = false;
        const Symbol * m_symbol = nullptr;
        std::string_view m_source_chars;

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

    class Lexer
    {
    public:

        Lexer(std::string_view i_source);

        /** Returns the current token without accepting it. Heading spaces are ignored */
        const Token & GetCurrentToken() const { return m_curr_token; }

        /** If after skipping spaces the current token matches the provided symbol id,
            accepets and returns it. The heading spaces can contain line-breaks. */
        std::optional<Token> TryAccept(SymbolId i_symbol_id);

        /** If after skipping spaces the current token matches the provided symbol id,
            accepets and returns it. The heading spaces cannot contain line-breaks. */
        std::optional<Token> TryAcceptInline(SymbolId i_symbol_id);

        /** Moves to the next token, and returns it. Heading spaces are ignored. */
        const Token & NextToken();

        bool IsSourceOver() const;

        /** Returns the all source, as passed to the constructor. */
        std::string_view GetWholeSource() const { return m_whole_source; }

    private:
        Token m_curr_token;
        std::string_view m_remaining_source;
        std::string_view m_whole_source;
    };

    template <> struct CharWriter<Lexer>
    {
        void operator() (CharBufferView & i_dest, const Lexer & i_source) noexcept;
    };

    bool IsSpace(uint32_t i_char);
    bool IsDigit(uint32_t i_char);
    bool IsAlpha(uint32_t i_char);
    bool IsAlphaOrUnderscore(uint32_t i_char);

} // namespace djup
