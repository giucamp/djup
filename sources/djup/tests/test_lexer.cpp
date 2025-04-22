
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <private/lexer.h>
#include <tests/test_utils.h>

namespace djup
{
    namespace tests
    {
        void Lexer()
        {
            Print("Test: djup - Lexer...");

            {
                djup::Lexer lexer("");
                CORE_EXPECTS(lexer.IsSourceOver());
            }

            {
                djup::Lexer lexer(" \n\t ");
                CORE_EXPECTS(lexer.IsSourceOver());
            }

            {
                djup::Lexer lexer("*1234.3e-10 1234.3e+10 {");
                CORE_EXPECTS(lexer.GetCurrentToken().m_symbol_id == SymbolId::Mul);
                CORE_EXPECTS(lexer.NextToken().m_symbol_id == SymbolId::NumericLiteral);

                CORE_EXPECTS(!lexer.IsSourceOver());
                CORE_EXPECTS_EQ(lexer.GetCurrentToken().m_source_chars, "1234.3e-10");
                lexer.NextToken();
                
                CORE_EXPECTS_EQ(lexer.GetCurrentToken().m_source_chars, "1234.3e+10");
                lexer.NextToken();

                CORE_EXPECTS(lexer.TryAccept(SymbolId::LeftBrace));
                CORE_EXPECTS(lexer.IsSourceOver());
            }

            // test white simmetry rule on binary operators
            {
                djup::Lexer lexer("5 + true");
                CORE_EXPECTS(lexer.TryAccept(SymbolId::NumericLiteral));
                CORE_EXPECTS(lexer.TryAccept(SymbolId::BinaryPlus));
                CORE_EXPECTS(lexer.TryAccept(SymbolId::BoolLiteral));
            }
            {
                djup::Lexer lexer("5\t + \ttrue");
                CORE_EXPECTS(lexer.TryAccept(SymbolId::NumericLiteral));
                CORE_EXPECTS(lexer.TryAccept(SymbolId::BinaryPlus));
                CORE_EXPECTS(lexer.TryAccept(SymbolId::BoolLiteral));
            }
            {
                djup::Lexer lexer("5 +true");
                CORE_EXPECTS(lexer.TryAccept(SymbolId::NumericLiteral));
                CORE_EXPECTS(lexer.TryAccept(SymbolId::UnaryPlus));
                CORE_EXPECTS(lexer.TryAccept(SymbolId::BoolLiteral));
            }
            {
                djup::Lexer lexer("5\t+ true");
                CORE_EXPECTS(lexer.TryAccept(SymbolId::NumericLiteral));
                CORE_EXPECTS(lexer.TryAccept(SymbolId::UnaryPlus));
                CORE_EXPECTS(lexer.TryAccept(SymbolId::BoolLiteral));
            }

            // test m_follows_line_break
            {
                djup::Lexer lexer("\n5\r\n\t+ true");
                CORE_EXPECTS(lexer.TryAccept(SymbolId::NumericLiteral)->m_follows_line_break);
                CORE_EXPECTS(lexer.TryAccept(SymbolId::UnaryPlus)->m_follows_line_break);
                CORE_EXPECTS(!lexer.TryAccept(SymbolId::BoolLiteral)->m_follows_line_break);
            }

            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup
