
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/pattern/discrimination_net.h>
#include <private/pattern/substitution_graph.h>
#include <core/diagnostic.h>
#include <fstream>
#include <filesystem>

namespace djup
{
    namespace tests
    {
        namespace
        {
            void SaveGraph(std::string i_dir, std::string i_name, std::string i_dot)
            {
                std::string dot_file_path(i_dir + i_name);
                std::ofstream(dot_file_path) << i_dot;
                std::string cmd = ToString("\"", "\"C:\\Program Files\\Graphviz\\bin\\dot.exe\"", " -T png -O ", dot_file_path, "\"");
                int res = std::system(cmd.c_str());
                if (res != 0)
                    Error("The command ", cmd, " returned ", res);

                std::filesystem::remove(dot_file_path);
            }
        }

        void Pattern()
        {
            Print("Test: djup - Pattern Matching...");
            
            pattern::DiscriminationNet discrimination_net;

            // discrimination_net.AddPattern(1, "g(3 f(1, 2, real x, 4), 8)");

            // discrimination_net.AddPattern(2, "g(3 f(1, real y, 4), 11)");

            // discrimination_net.AddPattern(1, "f(1, 2, real x, 4)");

            // discrimination_net.AddPattern(2, "f(1, real y, 4)");


            /*discrimination_net.AddPattern(10, "f(1, h(1, real x..., 55, real y), g(real z, 7, real w), 7)");


            discrimination_net.AddPattern(11, "f(1, h(1, real x..., 55, real y), g(real z, 7), 7)");

            discrimination_net.AddPattern(12, "f(1, h(3, real x..., 55, real y), g(real z, 7), 7)");*/

            //discrimination_net.AddPattern(4, "f(1, real y, g(real z, 1, 2, 3), 7, 2)");

            discrimination_net.AddPattern(1, "g(3 z(real r) p(real) 5)");
            discrimination_net.AddPattern(2, "g(3 m(real r) p(real) 7)");
            discrimination_net.AddPattern(3, "g(3 m(real r, real r1) p(real) 7)");
            bool save_it = true;
            if (save_it)
            {
                SaveGraph("D:\\repos\\djup\\tests\\", "discr", discrimination_net.ToDotLanguage("discr"));
            }

            for (auto file : std::filesystem::directory_iterator("D:\\repos\\djup\\tests\\subst"))
                if (file.is_regular_file())
                    std::filesystem::remove(file.path());

            pattern::SubstitutionGraph substitution_graph(discrimination_net);

            int step = 0;
            substitution_graph.FindMatches("g(3 z(88) p(2) 5)", [&] {
                std::string name = step == 0 ? "Initial" : ToString("Step_", step);
                SaveGraph("D:\\repos\\djup\\tests\\subst\\", name, substitution_graph.ToDotLanguage(name));
                step++;
            });

            int h = 0;

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
