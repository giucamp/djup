
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/namespace.h>
#include <core/diagnostic.h>
#include <fstream>

#define DBG_CREATE_GRAPHVIZ_SVG         1
#define DBG_GRAPHVIZ_EXE                "\"C:\\Program Files\\Graphviz\\bin\\dot.exe\""
#define DBG_DEST_DIR                    "D:\\repos\\djup\\tests\\"

extern bool g_enable_graphviz;

namespace djup
{
    namespace tests
    {
        void OldPattern()
        {
            Print("Test: djup - Old Pattern Matching...");

            {
                //g_enable_graphviz = true;
                auto target =  "g(f(1 2 3 4 5))"_t;
                auto pattern = "g(f(1 real x... real y...)...)"_t;
                size_t solutions = PatternMatchingCount(target, pattern);
                CORE_EXPECTS(solutions == 5);
            }

            {
                //g_enable_graphviz = true;
                auto target =  "g(f(1 2 3 4 5), f(1 2 5 6 7 8 9))"_t;
                auto pattern = "g(f(1 real x... real y...)...)"_t;
                size_t solutions = PatternMatchingCount(target, pattern);
                CORE_EXPECTS(solutions == 35);
            }

            {
                // g_enable_graphviz = true;
                auto target =  "Add(1 2 3 4 5)"_t;
                auto pattern = "Add(3 2 1 any y any x)"_t;
                size_t solutions = PatternMatchingCount(target, pattern);
                CORE_EXPECTS(solutions == 1);
            }

            {
                auto target =  "If(true, 1, true, 1, false, 2, 5)"_t;
                auto pattern = "If( (bool c, real v)..., real def)"_t;
                size_t solutions = PatternMatchingCount(target, pattern);
                CORE_EXPECTS(solutions == 1);
            }

            {
                // g_enable_graphviz = true;
                auto target = "Add(1 2 3 Cos(4) Sin(5))"_t;
                auto pattern = "Add(3 2 1 Sin(real x) Cos(real y))"_t;
                size_t solutions = PatternMatchingCount(target, pattern);
                CORE_EXPECTS(solutions == 1);
            }

            /*{
                // g_enable_graphviz = true;
                auto target =  "Add(1 9 2 3 4 5 6 7)"_t;
                auto pattern = "Add(1 2 real x real y 7)"_t;
                size_t solutions = PatternMatchingCount(target, pattern);
                CORE_EXPECTS(solutions == 3);
            }*/

            {
                //g_enable_graphviz = true;
                auto target =  "MatMul(1 2 3 4 5 6 7)"_t;
                auto pattern = "MatMul(1 2 real x real y 7)"_t;
                size_t solutions = PatternMatchingCount(target, pattern);
                CORE_EXPECTS(solutions == 3);
            }

            {
                //g_enable_graphviz = true;
                auto target = "3"_t;
                auto pattern = "real y"_t;
                size_t solutions = PatternMatchingCount(target, pattern);
                CORE_EXPECTS(solutions == 1);
            }

            {
                auto target =  "2"_t;
                auto pattern = "2"_t;
                size_t solutions = PatternMatchingCount(target, pattern);
                CORE_EXPECTS(solutions == 1);
            }

            {
                auto target =  "f(1 2 3)"_t;
                auto pattern = "f(1 2 3)"_t;
                size_t solutions = PatternMatchingCount(target, pattern);
                CORE_EXPECTS(solutions == 1);
            }

            {
                auto target =  "f(1 2 3)"_t;
                auto pattern = "f(real x..., real y...)"_t;
                size_t solutions = PatternMatchingCount(target, pattern);
                CORE_EXPECTS(solutions == 4);
            }

            {
                auto target =  "f(1, 2, 3,      4)"_t;
                auto pattern = "f(1, real x..., 5)"_t;
                auto substitution = "g(1, 2, Add(y...)..., 7)"_t;
                size_t solutions = PatternMatchingCount(target, pattern);
                CORE_EXPECTS(solutions == 0);
            }

            {
                auto target =  "f(Sin(1, 2, 3))"_t;
                auto pattern = "f(Sin(real x..., real y..., real z...))"_t;
                size_t solutions = PatternMatchingCount(target, pattern);
                CORE_EXPECTS(solutions == 10);
            }

            {
                //g_enable_graphviz = true;
                auto target =  "f(Cos(2,4), Sin(1, 2, 3), Sin(5, 6, 7, 8))"_t;
                auto pattern = "f(Cos(2,4), Sin(real x..., real y...), Sin(real z..., real w...))"_t;
                size_t solutions = PatternMatchingCount(target, pattern);
                CORE_EXPECTS(solutions == 20);
                g_enable_graphviz = false;
            }

            {
                // g_enable_graphviz = true;
                auto target =  "Sin(1 2 3 4 5)"_t;
                auto pattern = "Sin(1 real x... 4 5)"_t;
                auto match = Match(target, pattern);
                CORE_EXPECTS(match.m_substitutions.size() == 1);
            }

            {
                // g_enable_graphviz = true;
                auto target =  "MatMul(1 2 77 3 4 5 6 7)"_t;
                auto pattern = "MatMul(1 real x 3 4 real y 6 7)"_t;
                size_t solutions = PatternMatchingCount(target, pattern);
                CORE_EXPECTS(solutions == 1);
            }

            {
                //g_enable_graphviz = true;
                auto target =  "MatMul(1 2 3 4 5 6 7)"_t;
                auto pattern = "MatMul(1 real x 3 4 real y 6 7)"_t;
                size_t solutions = PatternMatchingCount(target, pattern);
                CORE_EXPECTS(solutions == 1);
            }

            {
                //g_enable_graphviz = true;
                auto target =  "Sin(f(1 2), f(4 5 6), f(7 8 9 1), f(11 12 13))"_t;
                auto pattern = "Sin(f(real x..., real y...)..., f(real z..., real w...)..., f(real u..., real p...)...)"_t;
                size_t solutions = PatternMatchingCount(target, pattern);
                CORE_EXPECTS(solutions == 3600);
            }

            {
                //g_enable_graphviz = true;
                auto target =  "f(Sin(1, 2, 3, 4), Sin(5, 3, 6, 7, 8, 9))"_t;
                auto pattern = "f(Sin(real x..., 3, real y...)...)"_t;
                size_t solutions = PatternMatchingCount(target, pattern);
                auto match = Match(target, pattern);
                CORE_EXPECTS(solutions == 1);
            }

            {
                auto target =  "f(Sin(1, 2, 3), Sin(5, 6, 7, 8))"_t;
                auto pattern = "f(Sin(real x..., 2, real y...)...)"_t;
                size_t solutions = PatternMatchingCount(target, pattern);
                CORE_EXPECTS(solutions == 0);
            }

            {
                // g_enable_graphviz = true;
                auto target =  "f(1, 2, Sin(1 + Add(4, 3)), Sin(1 + Add(5, 7, 9)), 3)"_t;
                auto pattern = "f(1, 2, Sin(1 + Add(real y...))...,         3)"_t;
                size_t solutions = PatternMatchingCount(target, pattern);
                CORE_EXPECTS(solutions == 1);
            }

            {
                auto target =  "f(1, 2, Sin(4), Sin(5), 3)"_t;
                auto pattern = "f(1, 2, Sin(real x)...,     3)"_t;
                auto substitution = "g(1, 2, x..., 7, y...)"_t;
                size_t solutions = PatternMatchingCount(target, pattern);
                CORE_EXPECTS(solutions == 1);
            }


            {
                auto target = "f(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15)"_t;
                auto pattern = "f(1, 2, real x..., 6, 7, 8, real y..., 12, 13, 14, 15)"_t;
                auto substitution = "g(1, 2, x..., 7, y...)"_t;
                size_t solutions = PatternMatchingCount(target, pattern);
                CORE_EXPECTS(solutions == 1);
            }

            Tensor t = "1";

            auto s = ToSimplifiedStringForm("0 * real");

            auto s1 = ToSimplifiedStringForm("0 * Add(real x?, 4)..");

            auto s2 = ToSimplifiedStringForm("f((5 real a)...)");

            auto s3 = ToSimplifiedStringForm("g((4 a)...)");

            R"(
                a = 4 + 6
            )"_t;

            CORE_EXPECTS(Is("0", "int"));
            CORE_EXPECTS(Is("0", "real"));
            CORE_EXPECTS(Is("false", "bool"));
            CORE_EXPECTS(!Is("true", "int"));
            CORE_EXPECTS(!Is("true", "real"));
            CORE_EXPECTS(!Is("1.2", "int"));

            Namespace test_namespace("Test", Namespace::Root());

            /*test_namespace.AddSubstitutionAxiom("f(real x1, real x2, real x3, real x4)", "5");
            test_namespace.AddSubstitutionAxiom("f(real x1, real x2, real x6, real x7, real x8..., real x9)", "0");

            
            test_namespace.AddSubstitutionAxiom("2+3", "5");
            test_namespace.AddSubstitutionAxiom("0 * real",             "0");
            test_namespace.AddSubstitutionAxiom("f(Sin(5, real a))",    "g((4, a)...)");
            test_namespace.AddSubstitutionAxiom("f(Sin(5, 7, real a...))",    "g((4, 6, a)...)");
            test_namespace.AddSubstitutionAxiom("f(5, 12, real a)",    "g((4, 6, 12, a)...)");
            test_namespace.AddSubstitutionAxiom("real x + real y + 5", "z");


            test_namespace.AddSubstitutionAxiom("f((3+1), 4, real x..., 5, 6)",    "g((4, a)...)");
            test_namespace.AddSubstitutionAxiom("f((3+1), 4, real x..., 9)",    "g((4, a)...)");*/

            auto str = test_namespace.SubstitutionGraphToDotLanguage(); 
            bool save_it = false;
            if (save_it)
            {
                std::string dot_file_path(std::string(DBG_DEST_DIR) + "Subst.txt");
                std::ofstream(dot_file_path) << str;
                std::string cmd = ToString("\"", DBG_GRAPHVIZ_EXE, " -T png -O ", dot_file_path, "\"");
                int res = std::system(cmd.c_str());
                if (res != 0)
                    Error("The command ", cmd, " returned ", res);
            }

            /*CORE_EXPECTS(AlwaysEqual(test_namespace.Canonicalize("2+3"), "5"));
            CORE_EXPECTS(AlwaysEqual(test_namespace.Canonicalize("0*7"), "0"));*/

            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup
