
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <private/namespace.h>
#include <private/pattern/discrimination_tree.h>
#include <private/pattern/substitution_graph.h>
#include <private/pattern/pattern_info.h>
#include <tests/test_utils.h>
#include <fstream>
#include <filesystem>

namespace djup
{
    namespace tests
    {
        void Pattern()
        {
            Print("Test: djup - Pattern Matching...");

            const std::filesystem::path artifact_path = GetArtifactPath("test_pattern");

            // create or clean artifact path
            if (std::filesystem::exists(artifact_path))
            {
                for (auto file : std::filesystem::recursive_directory_iterator(artifact_path))
                    if (file.is_regular_file())
                        std::filesystem::remove(file.path());
            }
            else
            {
                std::filesystem::create_directories(artifact_path);
            }

            CORE_EXPECTS_EQ(ToSimplifiedString("4"_t), "4");

#if 1
            // pattern 1
            {
                pattern::DiscriminationTree discrimination_net;
                discrimination_net.AddPattern(2, "g(1 2 3 any a any b any c)");
                static bool save_it = false;
                if (save_it)
                {
                    discrimination_net.ToGraphWiz("discrimination").SaveAsImage(artifact_path / "discr.png");
                }

                pattern::SubstitutionGraph substs_graph(discrimination_net);
                substs_graph.FindMatches(*GetStandardNamespace(), "g(1 2 3 4 5 6)"_t);
                CORE_EXPECTS(substs_graph.GetSolutions().size() == 1);
                const auto& solution = substs_graph.GetSolutions()[0];
                const auto& substitutions = solution.m_substitutions.GetSubstitutions();
                CORE_EXPECTS(substitutions.size() == 3);
                
                CORE_EXPECTS_EQ(substitutions[0].m_identifier_name, "a");
                CORE_EXPECTS(AlwaysEqual(substitutions[0].m_value, "4"_t));

                CORE_EXPECTS_EQ(substitutions[1].m_identifier_name, "b");
                CORE_EXPECTS(AlwaysEqual(substitutions[1].m_value, "5"_t));

                CORE_EXPECTS_EQ(substitutions[2].m_identifier_name, "c");
                CORE_EXPECTS(AlwaysEqual(substitutions[2].m_value, "6"_t));
            }

            // pattern 2
            {
                pattern::DiscriminationTree discrimination_net;
                discrimination_net.AddPattern(2, "g(1 2 3 any a any b any a)");

                pattern::SubstitutionGraph substs_graph(discrimination_net);
                substs_graph.FindMatches(*GetStandardNamespace(), "g(1 2 3 4 5 6)"_t);
                CORE_EXPECTS(substs_graph.GetSolutions().size() == 0);
            }

            // pattern 3
            {
                Tensor pattern = "g(1 2 3 f(real a h(real b)) real c)"_t;

                pattern::DiscriminationTree discrimination_net;
                discrimination_net.AddPattern(2, pattern);

                pattern::SubstitutionGraph substs_graph(discrimination_net);
                Tensor target = "g(1 2 3 f(4 h(5)) 6)"_t;
                substs_graph.FindMatches(*GetStandardNamespace(), "g(1 2 3 f(4 h(5)) 6)"_t);
                CORE_EXPECTS(substs_graph.GetSolutions().size() == 1);
                const auto& solution = substs_graph.GetSolutions()[0];
                const auto& substitutions = solution.m_substitutions.GetSubstitutions();

                Tensor after_sub = pattern::ApplySubstitutions(pattern, substitutions);

                CORE_EXPECTS(AlwaysEqual(after_sub, target));

                CORE_EXPECTS(substitutions.size() == 3);
            }
            
            // tree 1
            {
                pattern::DiscriminationTree discrimination_net;            
                discrimination_net.AddPattern(2, "g(1 2 3 any a any b any c)");
                discrimination_net.AddPattern(2,  "g(3 z(real r)... p(real) 6)");
                discrimination_net.AddPattern(21, "g(3 w(real r)... p(real) 6)");
                discrimination_net.AddPattern(22, "g(3 w(1 2 3 x)... p(real) 6)");
                discrimination_net.AddPattern(3, "g(3 m(real r) p(real) 7)");
                discrimination_net.AddPattern(4, "g(3 m(real r) w(real) 3)");
                discrimination_net.AddPattern(5, "Func(1 2 3)");
                discrimination_net.AddPattern(6, "f(1, 2, real x..., 6, 7, 8, real y..., 12, 13, 14, 15)");
                //discrimination_net.AddPattern(100, "g( w(x(y))? )");
                //discrimination_net.AddPattern(101, "g( w(x(y))... )");
                static bool save_it = false;
                if (save_it)
                {
                    discrimination_net.ToGraphWiz("discr").SaveAsImage(artifact_path / "discr.png");
                }

                // substitutions
                pattern::SubstitutionGraph substitution_graph(discrimination_net);
                int step = 0;
                std::string target = "g(1 2 3 4 5 6)";
            
                auto callback = [&] {
                    std::string name = step == 0 ? "Initial" : ToString("Step_", step);
                    substitution_graph.ToDotGraphWiz(name).SaveAsImage(
                        artifact_path / (name + ".png") );
                    step++;
                };

                substitution_graph.FindMatches(*GetStandardNamespace(), target.c_str(), callback);
            }

            // tree 2
            {
                pattern::DiscriminationTree discrimination_net;
                discrimination_net.AddPattern(2, "g(1 2 3 any a any b any c)");
                static bool save_it = false;
                if (save_it)
                {
                    discrimination_net.ToGraphWiz("discr").SaveAsImage(artifact_path / "discr.png");
                }

                // substitutions
                pattern::SubstitutionGraph substitution_graph(discrimination_net);
                int step = 0;
                std::string target = "g(1 2 3 4 5 6)";

                auto callback = [&] {
                    std::string name = step == 0 ? "Initial" : ToString("Step_", step);
                    substitution_graph.ToDotGraphWiz(name).SaveAsImage(
                        artifact_path / (name + ".png") );
                    step++;
                };
            }

#endif

            // tree 3
            {
                Tensor pattern = "f(1 2 real x... 7 8 9)";
                Tensor pattern2 = "f(1 2 g(real y)... 7 8 9)";
                Tensor target = "f(1 2 3 4 5 6 7 8 9)";

                pattern::DiscriminationTree discrimination_net;
                discrimination_net.AddPattern(1, pattern);
                discrimination_net.AddPattern(2, pattern2);
                static bool save_it = true;
                if (save_it)
                {
                    discrimination_net.ToGraphWiz("discr").SaveAsImage(artifact_path / "discr.png");
                }

                // substitutions
                pattern::SubstitutionGraph substitution_graph(discrimination_net);
                int step = 0;                

                auto callback = [&] {
                    std::string name = step == 0 ? "Initial" : ToString("Step_", step);
                    substitution_graph.ToDotGraphWiz(name).SaveAsImage(artifact_path / (name + ".png"));
                    step++;
                };

                substitution_graph.FindMatches(*GetStandardNamespace(), target, callback);

                CORE_EXPECTS(substitution_graph.GetSolutions().size() == 1);
                const auto& solution = substitution_graph.GetSolutionAt(0);
                const auto& substitutions = solution.m_substitutions.GetSubstitutions();

                Tensor after_sub = pattern::ApplySubstitutions(pattern, substitutions);

                CORE_EXPECTS(AlwaysEqual(after_sub, Tensor(target)));
            }

            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup
