
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
                descr.m_save_graphs = false;
                descr.m_pattern = pattern;
                descr.m_target = target;
                descr.m_expected_solutions = 1;
                O2oPatternTest(descr);
            }



            {
                auto target = "g(1 2 3 f(4 h(5)) 6)";
                auto pattern = "g(1 2 3 f(real a h(real b)) real c)";
                O2oPatternTestDescr descr;
                descr.m_test_name = "pattern_5";
                descr.m_save_graphs = false;
                descr.m_pattern = pattern;
                descr.m_target = target;
                descr.m_expected_solutions = 1;
                O2oPatternTest(descr);
            }

            {
                auto target = "g(3 z(1) z(2) z(3) p(10) 6)";
                auto pattern = "g(3 z(real r)... p(real) 6)";
                O2oPatternTestDescr descr;
                descr.m_test_name = "pattern_6";
                descr.m_save_graphs = false;
                descr.m_pattern = pattern;
                descr.m_target = target;
                descr.m_expected_solutions = 1;
                O2oPatternTest(descr);
            }

            {
                auto target = "f(1 2 5 6 7 8 9)"_t;
                auto pattern = "f(1 real x...)"_t;
                O2oPatternTestDescr descr;
                descr.m_test_name = "pattern_7";
                descr.m_save_graphs = false;
                descr.m_pattern = pattern;
                descr.m_target = target;
                descr.m_expected_solutions = 1;
                O2oPatternTest(descr);
            }
#endif

            {
                auto target = "g(f(1 2 3 4 5), f(1 2 5 6 7 8 9))"_t;
                auto pattern = "g(f(1 real x...)...)"_t;
                O2oPatternTestDescr descr;
                descr.m_test_name = "pattern_8";
                descr.m_save_graphs = false;
                descr.m_pattern = pattern;
                descr.m_target = target;
                descr.m_expected_solutions = 1;
                O2oPatternTest(descr);
            }

#if 0
            {
                auto target = "g(f(1 2 3 4 5), f(1 2 5 6 7 8 9))"_t;
                auto pattern = "g(f(1 real x... real y...)...)"_t;
                O2oPatternTestDescr descr;
                descr.m_test_name = "pattern_9";
                descr.m_save_graphs = false;
                descr.m_pattern = pattern;
                descr.m_target = target;
                descr.m_expected_solutions = 35; /* = 7*5, cartesian product 
                    between the replacements of z and y */
                O2oPatternTest(descr);
            }

            {
                auto target = "Add(1 2 3 4 5)"_t;
                auto pattern = "Add(3 2 1 any y any x)"_t;
                O2oPatternTestDescr descr;
                descr.m_test_name = "pattern_8";
                descr.m_save_graphs = false;
                descr.m_pattern = pattern;
                descr.m_target = target;
                descr.m_expected_solutions = 1;
                O2oPatternTest(descr);
            }

            {
                auto target = "If(true, 1, true, 1, false, 2, 5)"_t;
                auto pattern = "If( (bool c, real v)..., real def)"_t;
                O2oPatternTestDescr descr;
                descr.m_test_name = "pattern_8";
                descr.m_save_graphs = false;
                descr.m_pattern = pattern;
                descr.m_target = target;
                descr.m_expected_solutions = 1;
                O2oPatternTest(descr);
            }

            {
                auto target = "Add(1 2 3 Cos(4) Sin(5))"_t;
                auto pattern = "Add(3 2 1 Sin(real x) Cos(real y))"_t;
                O2oPatternTestDescr descr;
                descr.m_test_name = "pattern_8";
                descr.m_save_graphs = false;
                descr.m_pattern = pattern;
                descr.m_target = target;
                descr.m_expected_solutions = 1;
                O2oPatternTest(descr);
            }

            /*{
                auto target =  "Add(1 9 2 3 4 5 6 7)"_t;
                auto pattern = "Add(1 2 real x real y 7)"_t;
                size_t solutions = PatternMatchingCount(target, pattern);
                CORE_EXPECTS(solutions == 3);
            }*/

            {
                auto target = "MatMul(1 2 3 4 5 6 7)"_t;
                auto pattern = "MatMul(1 2 real x real y 7)"_t;
                O2oPatternTestDescr descr;
                descr.m_test_name = "pattern_8";
                descr.m_save_graphs = false;
                descr.m_pattern = pattern;
                descr.m_target = target;
                descr.m_expected_solutions = 3;
                O2oPatternTest(descr);
            }

            {
                auto target = "3"_t;
                auto pattern = "real y"_t;
                O2oPatternTestDescr descr;
                descr.m_test_name = "pattern_8";
                descr.m_save_graphs = false;
                descr.m_pattern = pattern;
                descr.m_target = target;
                descr.m_expected_solutions = 1;
                O2oPatternTest(descr);
            }

            {
                auto target = "2"_t;
                auto pattern = "2"_t;
                O2oPatternTestDescr descr;
                descr.m_test_name = "pattern_8";
                descr.m_save_graphs = false;
                descr.m_pattern = pattern;
                descr.m_target = target;
                descr.m_expected_solutions = 1;
                O2oPatternTest(descr);
            }

            {
                auto target = "f(1 2 3)"_t;
                auto pattern = "f(1 2 3)"_t;
                O2oPatternTestDescr descr;
                descr.m_test_name = "pattern_8";
                descr.m_save_graphs = false;
                descr.m_pattern = pattern;
                descr.m_target = target;
                descr.m_expected_solutions = 1;
                O2oPatternTest(descr);
            }

            {
                auto target = "f(1 2 3)"_t;
                auto pattern = "f(real x..., real y...)"_t;
                O2oPatternTestDescr descr;
                descr.m_test_name = "pattern_8";
                descr.m_save_graphs = false;
                descr.m_pattern = pattern;
                descr.m_target = target;
                descr.m_expected_solutions = 4;
                O2oPatternTest(descr);
            }

            {
                auto target = "f(1, 2, 3,      4)"_t;
                auto pattern = "f(1, real x..., 5)"_t;
                auto substitution = "g(1, 2, Add(y...)..., 7)"_t;
                O2oPatternTestDescr descr;
                descr.m_test_name = "pattern_8";
                descr.m_save_graphs = false;
                descr.m_pattern = pattern;
                descr.m_target = target;
                descr.m_expected_solutions = 0;
                O2oPatternTest(descr);
            }

            {
                auto target = "f(Sin(1, 2, 3))"_t;
                auto pattern = "f(Sin(real x..., real y..., real z...))"_t;
                O2oPatternTestDescr descr;
                descr.m_test_name = "pattern_8";
                descr.m_save_graphs = false;
                descr.m_pattern = pattern;
                descr.m_target = target;
                descr.m_expected_solutions = 10;
                O2oPatternTest(descr);
            }

            {
                auto target = "f(Cos(2,4), Sin(1, 2, 3), Sin(5, 6, 7, 8))"_t;
                auto pattern = "f(Cos(2,4), Sin(real x..., real y...), Sin(real z..., real w...))"_t;
                O2oPatternTestDescr descr;
                descr.m_test_name = "pattern_8";
                descr.m_save_graphs = false;
                descr.m_pattern = pattern;
                descr.m_target = target;
                descr.m_expected_solutions = 20;
                O2oPatternTest(descr);
            }

            {
                auto target = "Sin(1 2 3 4 5)"_t;
                auto pattern = "Sin(1 real x... 4 5)"_t;
                O2oPatternTestDescr descr;
                descr.m_test_name = "pattern_8";
                descr.m_save_graphs = false;
                descr.m_pattern = pattern;
                descr.m_target = target;
                descr.m_expected_solutions = 1;
                O2oPatternTest(descr);
            }

            {
                auto target = "MatMul(1 2 77 3 4 5 6 7)"_t;
                auto pattern = "MatMul(1 real x 3 4 real y 6 7)"_t;
                O2oPatternTestDescr descr;
                descr.m_test_name = "pattern_8";
                descr.m_save_graphs = false;
                descr.m_pattern = pattern;
                descr.m_target = target;
                descr.m_expected_solutions = 1;
                O2oPatternTest(descr);
            }

            {
                auto target = "MatMul(1 2 3 4 5 6 7)"_t;
                auto pattern = "MatMul(1 real x 3 4 real y 6 7)"_t;
                O2oPatternTestDescr descr;
                descr.m_test_name = "pattern_8";
                descr.m_save_graphs = false;
                descr.m_pattern = pattern;
                descr.m_target = target;
                descr.m_expected_solutions = 1;
                O2oPatternTest(descr);
            }

            {
                auto target = "Sin(f(1 2), f(4 5 6), f(7 8 9 1), f(11 12 13))"_t;
                auto pattern = "Sin(f(real x..., real y...)..., f(real z..., real w...)..., f(real u..., real p...)...)"_t;
                O2oPatternTestDescr descr;
                descr.m_test_name = "pattern_8";
                descr.m_save_graphs = false;
                descr.m_pattern = pattern;
                descr.m_target = target;
                descr.m_expected_solutions = 3600;
                O2oPatternTest(descr);
            }

            {
                auto target = "f(1 2 3 4)"_t;
                auto pattern = "f(real x... real y...)"_t;
                O2oPatternTestDescr descr;
                descr.m_test_name = "pattern_8";
                descr.m_save_graphs = false;
                descr.m_pattern = pattern;
                descr.m_target = target;
                descr.m_expected_solutions = 5;
                O2oPatternTest(descr);
            }

            {
                auto target = "f(Sin(1, 2, 3, 4), Sin(5, 3, 6, 7, 8, 9))"_t;
                auto pattern = "f(Sin(real x..., 3, real y...)...)"_t;
                O2oPatternTestDescr descr;
                descr.m_test_name = "pattern_8";
                descr.m_save_graphs = false;
                descr.m_pattern = pattern;
                descr.m_target = target;
                descr.m_expected_solutions = 1;
                O2oPatternTest(descr);
            }

            {
                auto target = "f(Sin(1, 2, 3), Sin(5, 6, 7, 8))"_t;
                auto pattern = "f(Sin(real x..., 2, real y...)...)"_t;
                O2oPatternTestDescr descr;
                descr.m_test_name = "pattern_8";
                descr.m_save_graphs = false;
                descr.m_pattern = pattern;
                descr.m_target = target;
                descr.m_expected_solutions = 1;
                O2oPatternTest(descr);
            }

            {
                auto target = "f(1, 2, Sin(1 + Add(4, 3)), Sin(1 + Add(5, 7, 9)), 3)"_t;
                auto pattern = "f(1, 2, Sin(1 + Add(real y...))...,         3)"_t;
                O2oPatternTestDescr descr;
                descr.m_test_name = "pattern_8";
                descr.m_save_graphs = false;
                descr.m_pattern = pattern;
                descr.m_target = target;
                descr.m_expected_solutions = 1;
                O2oPatternTest(descr);
            }

            {
                auto target = "f(1, 2, Sin(4), Sin(5), 3)"_t;
                auto pattern = "f(1, 2, Sin(real x)...,     3)"_t;
                auto substitution = "g(1, 2, x..., 7, y...)"_t;
                O2oPatternTestDescr descr;
                descr.m_test_name = "pattern_8";
                descr.m_save_graphs = false;
                descr.m_pattern = pattern;
                descr.m_target = target;
                descr.m_expected_solutions = 1;
                O2oPatternTest(descr);
            }


            {
                auto target = "f(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15)"_t;
                auto pattern = "f(1, 2, real x..., 6, 7, 8, real y..., 12, 13, 14, 15)"_t;
                auto substitution = "g(1, 2, x..., 7, y...)"_t;
                O2oPatternTestDescr descr;
                descr.m_test_name = "pattern_8";
                descr.m_save_graphs = false;
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
