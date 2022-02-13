
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/namespace.h>
#include <core/diagnostic.h>

extern bool g_enable_graphviz;

namespace djup
{
    namespace tests
    {
        void Pattern()
        {
            Print("Test: djup - Pattern Matching...");

            /*{
                auto target =  "2"_t;
                auto pattern = "2"_t;
                std::vector<PatternMatch> matches = Match(target, pattern);
                DJUP_EXPECTS(matches.size() == 1);
            }

            {
                auto target =  "f(1 2 3)"_t;
                auto pattern = "f(1 2 3)"_t;
                std::vector<PatternMatch> matches = Match(target, pattern);
                DJUP_EXPECTS(matches.size() == 1);
            }

            {
                auto target =  "f(1 2 3)"_t;
                auto pattern = "f(real x..., real y...)"_t;
                std::vector<PatternMatch> matches = Match(target, pattern);
                DJUP_EXPECTS(matches.size() == 4);
            }*/

            /*{
                auto target =  "f(1, 2, 3,      4)"_t;
                auto pattern = "f(1, real x..., 5)"_t;
                auto substitution = "g(1, 2, Add(y...)..., 7)"_t;
                std::vector<PatternMatch> matches = Match(target, pattern);
                DJUP_EXPECTS(matches.size() == 0);
            }*/

            /*{
                g_enable_graphviz = true;
                auto target =  "f(Sin(1, 2, 3))"_t;
                auto pattern = "f(Sin(real x..., real y..., real z...))"_t;
                std::vector<PatternMatch> matches = Match(target, pattern);
                DJUP_EXPECTS(matches.size() == 10);
            }*/

            /*{
                auto target =  "f(Sin(1, 2, 3), Sin(5, 6, 7, 8))"_t;
                auto pattern = "f(Sin(real x..., real y...), Sin(real z...))"_t;
                std::vector<PatternMatch> matches = Match(target, pattern);
                DJUP_EXPECTS(matches.size() == 20);
            }*/

            {
                g_enable_graphviz = true;
                auto target =  "Sin(f(1 2), f(4 5 6), f(7 8 9 1), f(11 12 13))"_t;
                auto pattern = "Sin(f(real x..., real y...)..., f(real z..., real w...)..., f(real u..., real p...)...)"_t;
                std::vector<PatternMatch> matches = Match(target, pattern);
                DJUP_EXPECTS(matches.size() == 0);
            }

            {
                g_enable_graphviz = true;
                auto target =  "f(Sin(1, 2, 3), Sin(2, 5, 6, 7, 8))"_t;
                auto pattern = "f(Sin(real x..., 2, real y...)...)"_t;
                std::vector<PatternMatch> matches = Match(target, pattern);
                DJUP_EXPECTS(matches.size() == 0);
            }

            {
                
                auto target =  "f(1, 2, Sin(1 + Add(4, 3)), Sin(1 + Add(5, 7, 9)), 3)"_t;
                auto pattern = "f(1, 2, Sin(1 + Add(real y...))...,         3)"_t;
                auto substitution = "g(1, 2, Add(y...)..., 7)"_t;
                std::vector<PatternMatch> matches = Match(target, pattern);
                DJUP_EXPECTS(matches.size() == 1);
                auto res = SubstitutePatternMatch(substitution, matches.front());
                auto s = ToSimplifiedStringForm(res);
                auto s1 = s;
            }

            {
                auto target =  "f(1, 2, Sin(4), Sin(5), 3)"_t;
                auto pattern = "f(1, 2, Sin(real x)...,     3)"_t;
                auto substitution = "g(1, 2, x..., 7, y...)"_t;
                std::vector<PatternMatch> matches = Match(target, pattern);
                DJUP_EXPECTS(matches.size() == 1);
                auto res = SubstitutePatternMatch(substitution, matches.front());
                auto s = ToSimplifiedStringForm(res);
                auto s1 = s;
            }


            {
                auto target = "f(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15)"_t;
                auto pattern = "f(1, 2, real x..., 6, 7, 8, real y..., 12, 13, 14, 15)"_t;
                auto substitution = "g(1, 2, x..., 7, y...)"_t;
                std::vector<PatternMatch> matches = Match(target, pattern);
                DJUP_EXPECTS(matches.size() == 1);
                auto res = SubstitutePatternMatch(substitution, matches.front());
                auto s = ToSimplifiedStringForm(res);
                auto s1 = s;
            }

            Tensor t = "1";

            auto s = ToSimplifiedStringForm("0 * real");

            auto s1 = ToSimplifiedStringForm("0 * Add(real x?, 4)..");

            auto s2 = ToSimplifiedStringForm("f((5 real a)...)");

            auto s3 = ToSimplifiedStringForm("g((4 a)...)");

            R"(
                a = 4 + 6
            )"_t;

            DJUP_EXPECTS(Is("0", "int"));
            DJUP_EXPECTS(Is("0", "real"));
            DJUP_EXPECTS(Is("false", "bool"));
            DJUP_EXPECTS(!Is("true", "int"));
            DJUP_EXPECTS(!Is("true", "real"));
            DJUP_EXPECTS(!Is("1.2", "int"));

            Namespace test_namespace("Test", Namespace::Root());
            test_namespace.AddSubstitutionAxiom("2+3",                  "5");
            test_namespace.AddSubstitutionAxiom("0 * real",             "0");
            test_namespace.AddSubstitutionAxiom("f((5, real a)...)",    "g((4, a)...)");


            DJUP_EXPECTS(AlwaysEqual(test_namespace.Canonicalize("2+3"), "5"));
            DJUP_EXPECTS(AlwaysEqual(test_namespace.Canonicalize("0*7"), "0"));

            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup
