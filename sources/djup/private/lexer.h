
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
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

        const Token & GetCurrentToken() const { return m_curr_token; }

        const Token & NextToken();

        std::optional<Token> TryAccept(SymbolId i_symbol_id);

        bool IsSourceOver() const;

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

} // namespace djup
