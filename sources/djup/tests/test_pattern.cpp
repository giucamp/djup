
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/pattern/discrimination_tree.h>
#include <private/pattern/substitution_graph.h>
#include <private/pattern/pattern_info.h>
#include <core/diagnostic.h>
#include <fstream>
#include <filesystem>

namespace djup
{
    namespace tests
    {
        namespace
        {
            const std::string test_dir = "C:\\repos\\djup\\tests\\";
            const std::string dot_exe = "\"C:\\Program Files\\Graphviz\\bin\\dot.exe\"";

            void SaveGraph(std::string i_dir, std::string i_name, std::string i_dot)
            {
                std::string dot_file_path(i_dir + i_name + ".txt");
                std::ofstream(dot_file_path) << i_dot;
                std::string cmd = ToString("\"", dot_exe, " -T png -O ", dot_file_path, "\"");
                int res = std::system(cmd.c_str());
                //std::filesystem::remove(dot_file_path);
                if (res != 0)
                    Error("The command ", cmd, " returned ", res);

                std::filesystem::remove(dot_file_path);
            }
        }

        void Pattern()
        {
            Print("Test: djup - Pattern Matching...");

            CORE_EXPECTS_EQ(ToSimplifiedStringForm("4"_t), "4");
            
            pattern::DiscriminationTree discrimination_net;
            
            discrimination_net.AddPattern(1, "g(1 2 x...)");
            
            //discrimination_net.AddPattern(1, "g(3 z(real r...)... p(real y) 5)");
            //discrimination_net.AddPattern(2, "g(3 z(real r)... p(real) 6)");
            //discrimination_net.AddPattern(3, "g(3 m(real r) p(real) 7)");
            //discrimination_net.AddPattern(4, "g(3 m(real r) w(real) 3)");
            //discrimination_net.AddPattern(5, "Func(1 2 3)");

            //discrimination_net.AddPattern(1, "g(1 2 Add(y...)... 7)";

            //discrimination_net.AddPattern(1, "g(3 z(real r)... 5)");


            //discrimination_net.AddPattern(1, "g(3 z(real r) 5)");
            //discrimination_net.AddPattern(2, "g(3 z(real r) 6)");

            //discrimination_net.AddPattern(1, "g(3 z(real r) 5)");

            bool save_it = true;
            if (save_it)
            {
                std::filesystem::create_directory(test_dir);
                SaveGraph(test_dir, "discr", discrimination_net.ToDotLanguage("discr"));
            }

            std::filesystem::create_directory(test_dir + "subst");
            for (auto file : std::filesystem::directory_iterator(test_dir + "subst"))
                if (file.is_regular_file())
                    std::filesystem::remove(file.path());

            pattern::SubstitutionGraph substitution_graph(discrimination_net);
            int step = 0;
            std::string target = "g(1 2 3)"; //"g(3 z(88) 5)"; // 

            auto callback = [&] {
                std::string name = step == 0 ? "Initial" : ToString("Step_", step);
                SaveGraph(test_dir + "subst\\", name, substitution_graph.ToDotLanguage(name));
                step++;
            };

            substitution_graph.FindMatches(target.c_str(), callback);

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

            /*CORE_EXPECTS(AlwaysEqual(test_namespace.Canonicalize("2+3"), "5"));
            CORE_EXPECTS(AlwaysEqual(test_namespace.Canonicalize("0*7"), "0"));*/

            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup
