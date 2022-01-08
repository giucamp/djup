
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/lexer.h>
#include <core/diagnostic.h>

namespace djup
{
    namespace tests
    {
        void Lexer()
        {
            Print("Test: djup - Lexer...");

            {
                djup::Lexer lexer("");
                DJUP_EXPECTS(lexer.IsSourceOver());
            }

            {
                djup::Lexer lexer("*1234.3e-10 1234.3e+10 ((");
                DJUP_EXPECTS(lexer.GetCurrentToken().m_symbol_id == SymbolId::Mul);
                DJUP_EXPECTS(lexer.NextToken().m_symbol_id == SymbolId::NumericLiteral);

                DJUP_EXPECTS(!lexer.IsSourceOver());
                DJUP_EXPECTS_EQ(lexer.GetCurrentToken().m_source_chars, "1234.3e-10");
                lexer.NextToken();
                
                DJUP_EXPECTS_EQ(lexer.GetCurrentToken().m_source_chars, "1234.3e+10");
                lexer.NextToken();

                DJUP_EXPECTS(lexer.TryAccept(SymbolId::BeginTuple));
                DJUP_EXPECTS(lexer.IsSourceOver());
            }

            // test white simmetry rule on binary operators
            {
                djup::Lexer lexer("5 + true");
                DJUP_EXPECTS(lexer.TryAccept(SymbolId::NumericLiteral));
                DJUP_EXPECTS(lexer.TryAccept(SymbolId::BinaryPlus));
                DJUP_EXPECTS(lexer.TryAccept(SymbolId::BoolLiteral));
            }
            {
                djup::Lexer lexer("5\t + \ttrue");
                DJUP_EXPECTS(lexer.TryAccept(SymbolId::NumericLiteral));
                DJUP_EXPECTS(lexer.TryAccept(SymbolId::BinaryPlus));
                DJUP_EXPECTS(lexer.TryAccept(SymbolId::BoolLiteral));
            }
            {
                djup::Lexer lexer("5 +true");
                DJUP_EXPECTS(lexer.TryAccept(SymbolId::NumericLiteral));
                DJUP_EXPECTS(lexer.TryAccept(SymbolId::UnaryPlus));
                DJUP_EXPECTS(lexer.TryAccept(SymbolId::BoolLiteral));
            }
            {
                djup::Lexer lexer("5\t+ true");
                DJUP_EXPECTS(lexer.TryAccept(SymbolId::NumericLiteral));
                DJUP_EXPECTS(lexer.TryAccept(SymbolId::UnaryPlus));
                DJUP_EXPECTS(lexer.TryAccept(SymbolId::BoolLiteral));
            }

            // test m_follows_line_break
            {
                djup::Lexer lexer("\n5\r\n\t+ true");
                DJUP_EXPECTS(lexer.TryAccept(SymbolId::NumericLiteral)->m_follows_line_break);
                DJUP_EXPECTS(lexer.TryAccept(SymbolId::UnaryPlus)->m_follows_line_break);
                DJUP_EXPECTS(!lexer.TryAccept(SymbolId::BoolLiteral)->m_follows_line_break);
            }

            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup
