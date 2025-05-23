
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <private/namespace.h>
#include <private/m2o_pattern/m2o_discrimination_tree.h>
#include <private/m2o_pattern/m2o_substitution_graph.h>
#include <private/m2o_pattern/m2o_pattern_info.h>
#include <tests/test_utils.h>
#include <fstream>
#include <filesystem>

namespace djup
{
    namespace tests
    {
        struct M2oPatternTestDescr
        {
            std::string m_test_name;
            std::vector<Tensor> m_patterns;
            Tensor m_target;
            size_t m_expected_solutions = std::numeric_limits<size_t>::max();
            bool m_save_graphs{ false };
        };

        void M2oPatternTest(const M2oPatternTestDescr & i_test_descr)
        {
            const std::string rel_artifact_path = "test_m2opattern/" + i_test_descr.m_test_name;
            const std::filesystem::path artifact_path = GetArtifactPath(rel_artifact_path);
            
            if (i_test_descr.m_save_graphs)
            {
                std::filesystem::create_directories(artifact_path);
            }

            m2o_pattern::DiscriminationTree discrimination_net;
            for (uint32_t i = 0; i < i_test_descr.m_patterns.size(); i++)
            {
                discrimination_net.AddPattern(i, i_test_descr.m_patterns[i]);
            }
            
            if (i_test_descr.m_save_graphs)
            {
                std::string graph_title = ToSimplifiedString(i_test_descr.m_target) + "\ndiscr";
                discrimination_net.ToGraphWiz(graph_title).SaveAsImage(artifact_path / "discr.png");
            }

            // substitutions
            m2o_pattern::SubstitutionGraph substitution_graph(discrimination_net);
            int step = 0;

            auto callback = [&] {
                if (i_test_descr.m_save_graphs)
                {
                    std::string name = step == 0 ? "Initial" : ToString("Step_", step);
                    std::string title = ToSimplifiedString(i_test_descr.m_target) + "\n" + name;
                    substitution_graph.ToDotGraphWiz(title).SaveAsImage(artifact_path / (name + ".png"));
                    step++;
                }
            };

            substitution_graph.FindMatches(*GetStandardNamespace(), i_test_descr.m_target, callback);

            CORE_EXPECTS_EQ(substitution_graph.GetSolutions().size(), i_test_descr.m_expected_solutions);

            if (i_test_descr.m_expected_solutions >= 1)
            {
                const auto& solution = substitution_graph.GetSolutionAt(0);
                const auto& substitutions = solution.m_substitutions.GetSubstitutions();

                Tensor after_sub = m2o_pattern::ApplySubstitutions(
                    i_test_descr.m_patterns[solution.m_pattern_id], substitutions);

                CORE_EXPECTS(AlwaysEqual(after_sub, i_test_descr.m_target));
            }
        }

        void M2oPattern()
        {
            Print("Test: djup - m2o Pattern Matching...");

            const std::filesystem::path artifact_path = GetArtifactPath("test_pattern");

            // create or clean artifact path
            if (std::filesystem::exists(artifact_path))
            {
                for (auto file : std::filesystem::directory_iterator(artifact_path))
                    std::filesystem::remove_all(file.path());
            }
            else
            {
                std::filesystem::create_directories(artifact_path);
            }

            /*TensorToGraph("f(1 2 Sin(a + g(b...))... + c 3 4)"_t)
                .SaveAsImage(artifact_path / "nested_pattern.png");*/

            // pattern 0
            {
                M2oPatternTestDescr descr;
                descr.m_test_name = "pattern_1";
                descr.m_save_graphs = true;
                descr.m_patterns = { 
                    "f(1 2 Sin(real x)... 3 4 Sin(y)...)"_t,
                    "f(1 2 Sin(real x)... 3 4 Cos(y)...)"_t,
                    "f(1 2 Cos(real x)... 3 4 Sin(y)...)"_t,
                    "f(1 2 Cos(real x)... 3 4 Cos(y)...)"_t,
                };
                descr.m_target = "g(1 2 Sin(10) Sin(11) 3 4 Cos(12) Cos(13))"_t;
                descr.m_expected_solutions = 1;
                M2oPatternTest(descr);
            }
#if 1
            // pattern 1
            {
                M2oPatternTestDescr descr;
                descr.m_test_name = "pattern_1";
                descr.m_save_graphs = true;
                descr.m_patterns = { "g(1 2 3 any a any b any c)"_t };
                descr.m_target = "g(1 2 3 4 5 6)"_t;
                descr.m_expected_solutions = 1;
                M2oPatternTest(descr);
            }

            // pattern 2
            {
                M2oPatternTestDescr descr;
                descr.m_test_name = "pattern_2";
                descr.m_save_graphs = false;
                descr.m_patterns = { "g(1 2 3 any a any b any a)"_t };
                descr.m_target = "g(1 2 3 4 5 6)"_t;
                descr.m_expected_solutions = 0;
                M2oPatternTest(descr);
            }

            // pattern 3
            {
                M2oPatternTestDescr descr;
                descr.m_test_name = "pattern_3";
                descr.m_save_graphs = false;
                descr.m_patterns = { "g(1 2 3 f(real a h(real b)) real c)"_t };
                descr.m_target = "g(1 2 3 f(4 h(5)) 6)"_t;
                descr.m_expected_solutions = 1;
                M2oPatternTest(descr);
            }
            
            // tree 1
            {
                M2oPatternTestDescr descr;
                descr.m_test_name = "tree_1";
                descr.m_save_graphs = false;
                descr.m_patterns = {
                    "g(1 2 3 any a any b any c)"_t,
                    "g(3 z(real r)... p(real) 6)"_t,
                    "g(3 w(real r)... p(real) 6)"_t,
                    "g(3 w(1 2 3 x)... p(real) 6)"_t,
                    "g(3 m(real r) p(real) 7)"_t,
                    "g(3 m(real r) w(real) 3)"_t,
                    "Func(1 2 3)"_t,
                    "f(1, 2, real x..., 6, 7, 8, real y..., 12, 13, 14, 15)"_t,
                };
                descr.m_target = "g(1 2 3 4 5 6)"_t;
                descr.m_expected_solutions = 1;
                M2oPatternTest(descr);
            }

            // tree 3
            {
                M2oPatternTestDescr descr;
                descr.m_test_name = "tree_3";
                descr.m_save_graphs = true;
                descr.m_patterns = {
                    "f(1 2 real x... 7 8 9)"_t,
                    "f(1 2 g(real y)... 7 8 9)"_t,
                };
                descr.m_target = "f(1 2 3 4 5 6 7 8 9)"_t;
                descr.m_expected_solutions = 1;
                M2oPatternTest(descr);
            }

#endif


            // tree 4
            {
                /*Tensor pattern = "f(g(x...)...)";
                //Tensor pattern2 = "f(1)";
                Tensor target = "f(g(1 2) g(3 4))";*/

                Tensor pattern = "f(real x...)";
                Tensor target = "f(1 2 3 4)";

                /*Tensor pattern =  "f(1 g(real a)...)";
                Tensor pattern2 = "f(1 w(real b)...)";
                Tensor target = "f(1 g(1) g(2) g(3))";*/
                
                m2o_pattern::DiscriminationTree discrimination_net;
                discrimination_net.AddPattern(1, pattern);
                //discrimination_net.AddPattern(2, pattern2);
                static bool save_it = true;
                if (save_it)
                {
                    std::string graph_title = ToSimplifiedString(target) + "\ndiscr";
                    discrimination_net.ToGraphWiz(graph_title).SaveAsImage(artifact_path / "discr.png");
                }

                // substitutions
                m2o_pattern::SubstitutionGraph substitution_graph(discrimination_net);
                int step = 0;

                auto callback = [&] {
                    if (save_it)
                    {
                        std::string name = step == 0 ? "Initial" : ToString("Step_", step);
                        std::string title = ToSimplifiedString(target) + "\n" + name;
                        substitution_graph.ToDotGraphWiz(title).SaveAsImage(artifact_path / (name + ".png"));
                        step++;
                    }
                };

                substitution_graph.FindMatches(*GetStandardNamespace(), target, callback);

                CORE_EXPECTS(substitution_graph.GetSolutions().size() == 1);
                const auto& solution = substitution_graph.GetSolutionAt(0);
                const auto& substitutions = solution.m_substitutions.GetSubstitutions();

                Tensor after_sub = m2o_pattern::ApplySubstitutions(pattern, substitutions);

                //CORE_EXPECTS(AlwaysEqual(after_sub, Tensor(target)));
            }

            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup
