
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
        void Pattern()
        {
            Print("Test: djup - Pattern Matching...");
            Namespace test_namespace("Test", Namespace::Root());

            test_namespace.AddSubstitutionAxiom("f(1, 2, real x, 4)", "5");

            test_namespace.AddSubstitutionAxiom("f(1, real y, 4)", "5");

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

            test_namespace.Canonicalize("f(1, 2, 3, 4)");

            /*
            test_namespace.AddSubstitutionAxiom("2+3", "5");
            test_namespace.AddSubstitutionAxiom("0 * real",             "0");
            test_namespace.AddSubstitutionAxiom("f(Sin(5, real a))",    "g((4, a)...)");
            test_namespace.AddSubstitutionAxiom("f(Sin(5, 7, real a...))",    "g((4, 6, a)...)");
            test_namespace.AddSubstitutionAxiom("f(5, 12, real a)",    "g((4, 6, 12, a)...)");
            test_namespace.AddSubstitutionAxiom("real x + real y + 5", "z");


            test_namespace.AddSubstitutionAxiom("f((3+1), 4, real x..., 5, 6)",    "g((4, a)...)");
            test_namespace.AddSubstitutionAxiom("f((3+1), 4, real x..., 9)",    "g((4, a)...)");*/

            /*DJUP_EXPECTS(AlwaysEqual(test_namespace.Canonicalize("2+3"), "5"));
            DJUP_EXPECTS(AlwaysEqual(test_namespace.Canonicalize("0*7"), "0"));*/

            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup
