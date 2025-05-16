
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <private/namespace.h>
#include <private/o2o_pattern/o2o_pattern_match.h>
#include <tests/test_utils.h>
#include <fstream>
#include <filesystem>

namespace djup
{
    namespace tests
    {
        struct O2oPatternTestDescr
        {
            std::string m_test_name;
            Tensor m_pattern;
            Tensor m_target;
            size_t m_expected_solutions = std::numeric_limits<size_t>::max();
            bool m_save_graphs{ false };
        };

        void O2oPatternTest(const O2oPatternTestDescr & i_test_descr)
        {
            const std::string rel_artifact_path = "test_o2opattern/" + i_test_descr.m_test_name;
            const std::filesystem::path artifact_path = GetArtifactPath(rel_artifact_path);
            const std::string artifact_path_string = artifact_path.string();

            // create or clean artifact path
            if (i_test_descr.m_save_graphs)
            {
                if (std::filesystem::exists(artifact_path))
                {
                    for (auto file : std::filesystem::directory_iterator(artifact_path))
                        std::filesystem::remove_all(file.path());
                }
                else
                {
                    std::filesystem::create_directories(artifact_path);
                }
            }

            o2o_pattern::Pattern pattern(*GetStandardNamespace(), i_test_descr.m_pattern);

            std::vector<o2o_pattern::MatchResult> solutions = pattern.MatchAll(i_test_descr.m_target,
                i_test_descr.m_save_graphs ? artifact_path_string.c_str() : nullptr);
            
            CORE_EXPECTS_EQ(solutions.size(), i_test_descr.m_expected_solutions);

            for (size_t solution_index = 0; solution_index < solutions.size(); ++solution_index)
            {
                Tensor after_sub = o2o_pattern::ApplySubstitutions(
                    i_test_descr.m_pattern, solutions[solution_index].m_substitutions);

                CORE_EXPECTS(AlwaysEqual(after_sub, i_test_descr.m_target));
            }
        }

        void O2oPattern()
        {
            Print("Test: djup - o2o Pattern Matching...");

#if 1
            // pattern 1
            {
                O2oPatternTestDescr descr;
                descr.m_test_name = "pattern_1";
                descr.m_save_graphs = false;
                descr.m_pattern = "g(1 2 3 any a any b any c)"_t;
                descr.m_target = "g(1 2 3 4 5 6)"_t;
                descr.m_expected_solutions = 1;
                O2oPatternTest(descr);
            }

            // pattern 2
            {
                O2oPatternTestDescr descr;
                descr.m_test_name = "pattern_2";
                descr.m_save_graphs = false;
                descr.m_pattern = "g(1 2 3 f(real a h(real b)) real c)"_t;
                descr.m_target = "g(1 2 3 f(4 h(5)) 6)"_t;
                descr.m_expected_solutions = 1;
                O2oPatternTest(descr);
            }

            // pattern 3
            {
                auto target = "f(1 2 3 4 5 6)"_t;
                auto pattern = "f(real x... real y...)"_t;

                O2oPatternTestDescr descr;
                descr.m_test_name = "pattern_3";
                descr.m_save_graphs = false;
                descr.m_pattern = pattern;
                descr.m_target = target;
                descr.m_expected_solutions = 7;
                O2oPatternTest(descr);
            }

            // pattern 4
            {
                auto target = "f(Sin(1) Sin(2) Sin(3) Sin(4))"_t;
                auto pattern = "f(Sin(real x)...)"_t;

                O2oPatternTestDescr descr;
                descr.m_test_name = "pattern_4";
                descr.m_save_graphs = true;
                descr.m_pattern = pattern;
                descr.m_target = target;
                descr.m_expected_solutions = 1;
                O2oPatternTest(descr);
            }
#endif

            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup
