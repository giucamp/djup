
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <private/lexer.h>
#include <core/diagnostic.h>
#include <string>
#include <cinttypes>

namespace djup
{
    bool IsSpace(uint32_t i_char)
    {
        if( i_char >= 0x0009 && i_char <= 0x000D )
            return true;

        if(i_char == 0x0020)
            return true;

        return false;
    }

    bool IsDigit(uint32_t i_char)
    {
        return i_char >= '0' && i_char <= '9';
    }

    // to support UTF-8 we consider alphabetic also any non-ansi char
    bool IsAlpha(uint32_t i_char)
    {
        return (i_char >= 'a' && i_char <= 'z') ||
            (i_char >= 'A' && i_char <= 'Z') ||
            i_char >= 0x7F;
    }

    bool IsAlphaNumOrUnderscore(uint32_t i_char)
    {
        return IsAlpha(i_char) || i_char == '_' || IsDigit(i_char);
    }

    namespace
    {
        bool StartsWith(std::string_view i_what, char i_with)
        {
            return !i_what.empty() && i_what.front() == i_with;
        }

        bool StartsWith(std::string_view i_what, std::string_view i_with)
        {
            return i_what.length() >= i_with.length() &&
                i_what.substr(0, i_with.length()) == i_with;
        }

        std::string_view ParseSpaces(std::string_view & io_source)
        {
            const char * beginning = io_source.data();
            while(!io_source.empty() && IsSpace(io_source.front()))
                io_source.remove_prefix(1);
            return std::string_view(beginning, io_source.data() - beginning);
        }

        bool TryParseString(std::string_view & io_source, std::string_view i_what)
        {
            if (StartsWith(io_source, i_what))
            {
                io_source = io_source.substr(i_what.length());
                return true;
            }

            return false;
        }

        bool TryParseWholeString(std::string_view & io_source, std::string_view i_what)
        {
            if (StartsWith(io_source, i_what))
            {
                std::string_view new_source = io_source.substr(i_what.length());
                if(new_source.empty() || !IsAlpha(new_source.front()))
                {
                    io_source = new_source;
                    return true;
                }
            }

            return false;
        }

        /* returns true if two sequences of spaces are simmetrical, that is
            if i_prefix == reverse(i_postfix). The sequence \r\n is considered
            a single space, and must not be reversed in the postix. */
        bool WhiteSimmetry(std::string_view i_prefix, std::string_view i_postfix)
        {
            if(i_prefix.length() != i_postfix.length())
                return false;

            size_t const length = i_prefix.length();
            for(size_t i = 0; i < length; i++)
            {
                if(i_prefix[i] == '\r' && i + 1 < length &&
                    i_prefix[i] == '\n')
                {
                    if(i_postfix[length - 2 - i] != '\r' ||
                        i_postfix[length - 1 - i] != '\n')
                        return false;
                    else
                        i++;
                }
                else if(i_prefix[i] != i_postfix[length - 1 - i])
                    return false;
            }
            return true;
        }

        Token ParseNumericLiteral(std::string_view & io_source)
        {
            DJUP_ASSERT(!io_source.empty());

            auto SkipDigits = [&]{
                while(!io_source.empty() && IsDigit(io_source.front()))
                    io_source.remove_prefix(1);
            };

            SkipDigits();

            if(StartsWith(io_source, '.'))
                io_source.remove_prefix(1);

            SkipDigits();

            if(StartsWith(io_source, 'e') || StartsWith(io_source, 'E'))
            {
                io_source.remove_prefix(1);

                if(StartsWith(io_source, '-'))
                    io_source.remove_prefix(1);
                else if(StartsWith(io_source, '+'))
                    io_source.remove_prefix(1);

                SkipDigits();
            }

            return Token{ SymbolId::NumericLiteral };
        }

        Token ParseName(std::string_view & io_source)
        {
            DJUP_ASSERT(!io_source.empty());
            DJUP_ASSERT(IsAlpha(io_source.front()) || io_source.front() == '_');

            while(!io_source.empty() && IsAlphaNumOrUnderscore(io_source.front()))
            {
                io_source.remove_prefix(1);
            }

            return Token{ SymbolId::Name };
        }

        Token ParseTokenImpl(std::string_view i_prefix_spaces, std::string_view & io_source)
        {
            for(const Symbol & symbol : g_alphabet)
            {
                if(symbol.IsBinaryOperator())
                {
                    // binary operator - enforce white space symmetry
                    std::string_view new_source = io_source;
                    if(TryParseString(new_source, symbol.m_chars))
                    {
                        /* the operator was accepted on a copy of source, so that we are able to
                            reject the matching if white simmetry is not respected. Now we do a 
                            read-ahead of the spaces following the operator, but even if we accept 
                            the operator we keep the spaces before the next token (that's why we save
                            another copy of the source if new_source_with_spaces). */
                        std::string_view new_source_with_spaces = new_source;
                        if(WhiteSimmetry(i_prefix_spaces, ParseSpaces(new_source)))
                        {
                            io_source = new_source_with_spaces;
                            return { symbol };
                        }
                    }
                }
                else
                {
                    // non-binary operator
                    if(TryParseString(io_source, symbol.m_chars))
                        return { symbol };
                }
            }

            if(io_source.empty())
                return { SymbolId::EndOfSource };
            else if(IsDigit(io_source.front()))
                return ParseNumericLiteral(io_source);
            else if(TryParseWholeString(io_source, "true") || TryParseWholeString(io_source, "false"))
                return Token{ SymbolId::BoolLiteral };
            else if(IsAlpha(io_source.front()) || io_source.front() == '_')
                return ParseName(io_source);
            else
                Error("Lexer - unexpected byte: ", io_source.front());
        }

        // range of source chars delimited by line-enders or source bounds
        struct Line { std::string_view m_chars; size_t m_number; };

        // finds the line containing the character pointed by i_at
        Line GetLineAt(std::string_view i_source, const char * i_at)
        {
            const char * curr_char = i_source.data();
            const char * const end_of_source = curr_char + i_source.size();

            if(i_at < curr_char || i_at > end_of_source)
                Error("GetLineAt - i_at is outside the source code");

            const char * beginning_of_line = curr_char;
            size_t line_number = 1;
            while(curr_char < i_at)
            {
                if(*curr_char == '\n')
                {
                    line_number++;
                    curr_char++;
                    beginning_of_line = curr_char;
                }
                else if(*curr_char == '\r' &&
                    curr_char + 1 < end_of_source &&
                    curr_char[1] == '\n')
                {
                    line_number++;
                    curr_char += 2;
                    beginning_of_line = curr_char;
                }
                else
                    curr_char++;
            }

            const char * end_of_line = curr_char;
            while(end_of_line < end_of_source && *end_of_line != '\n')
                end_of_line++;
            if(end_of_line > i_source.data() && end_of_line[-1] == '\r')
                end_of_line--;

            return {std::string_view(beginning_of_line, end_of_line - beginning_of_line), line_number};
        }
    }

    Lexer::Lexer(std::string_view i_source)
        : m_remaining_source(i_source), m_whole_source(i_source)
    {
        NextTokenImpl();
    }

    const Token & Lexer::NextToken()
    {
        DJUP_ASSERT(!IsSourceOver());
        NextTokenImpl();
        return m_curr_token;
    }

    void Lexer::NextTokenImpl()
    {
        auto const spaces = ParseSpaces(m_remaining_source);

        const char * first_char = m_remaining_source.data();
        Token token = ParseTokenImpl(spaces, m_remaining_source);
        token.m_source_chars = { first_char, static_cast<size_t>(m_remaining_source.data() - first_char) };
        token.m_follows_line_break = spaces.find_first_of('\n') != std::string_view::npos;
        m_curr_token = token;
    }

    std::optional<Token> Lexer::TryAccept(SymbolId i_symbol_id)
    {
        if(m_curr_token.m_symbol_id == i_symbol_id)
        {
            auto const result = m_curr_token;
            NextToken();
            return result;
        }
        else
            return { };
    }

    std::optional<Token> Lexer::TryAcceptInline(SymbolId i_symbol_id)
    {
        if(!m_curr_token.m_follows_line_break)
            return TryAccept(i_symbol_id);
        else
            return {};
    }

    Token Lexer::Accept(SymbolId i_symbol_id)
    {
        if(auto token = TryAccept(i_symbol_id))
            return *token;

        Error("Expected ", GetSymbolChars(i_symbol_id));
    }

    Token Lexer::AcceptInline(SymbolId i_symbol_id)
    {
        if(auto token = TryAcceptInline(i_symbol_id))
            return *token;

        Error("Expected ", GetSymbolChars(i_symbol_id), " in this line");
    }

    bool Lexer::IsSourceOver() const
    {
        return m_curr_token.m_symbol_id == SymbolId::EndOfSource;
    }
}

namespace core
{
    void CharWriter<djup::Lexer>::operator() (CharBufferView& i_dest, const djup::Lexer& i_lexer) noexcept
    {
        const char* at = i_lexer.GetCurrentToken().m_source_chars.data();
        djup::Line const line = djup::GetLineAt(i_lexer.GetWholeSource(), at);

        std::string const prefix = "(" + std::to_string(line.m_number) + "): ";
        i_dest << "\n" << prefix << line.m_chars;
        i_dest << "\n" << std::string(prefix.size() + (at - line.m_chars.data()), ' ') << '^';
    }
}
