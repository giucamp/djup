
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <private/m2o_pattern/pattern_info.h>
#include <tests/test_utils.h>

namespace djup
{
    namespace m2o_pattern
    {
        // defined in substitution_graph.cpp
        Range GetUsableCount(const ArgumentInfo& i_argument_info,
            uint32_t i_target_remaining_targets, uint32_t i_pattern_size);
    }

    namespace tests
    {
        void PatternInfo()
        {
            Print("Test: djup - Pattern Info...");

            using namespace m2o_pattern;

            const auto infinity = std::numeric_limits<int32_t>::max();

            {
                const m2o_pattern::PatternInfo info_1 = m2o_pattern::BuildPatternInfo("g(1 2 x...)");

                CORE_EXPECTS(info_1.m_flags == FunctionFlags{});
                CORE_EXPECTS_EQ(info_1.m_arguments_range, (Range{ 2, infinity }));

                CORE_EXPECTS_EQ(NumericCast<int32_t>(info_1.m_arguments_info.size()), 3);

                CORE_EXPECTS_EQ(info_1.m_arguments_info[0].m_cardinality, (Range{ 1, 1 }));
                CORE_EXPECTS_EQ(info_1.m_arguments_info[1].m_cardinality, (Range{ 1, 1 }));
                CORE_EXPECTS_EQ(info_1.m_arguments_info[2].m_cardinality, (Range{ 0, infinity }));
                CORE_EXPECTS_EQ(info_1.m_arguments_info[0].m_remaining, (Range{ 1, infinity }));
                CORE_EXPECTS_EQ(info_1.m_arguments_info[1].m_remaining, (Range{ 0, infinity }));
                CORE_EXPECTS_EQ(info_1.m_arguments_info[2].m_remaining, (Range{ 0, 0 }));

                #if DJUP_DEBUG_PATTERN_INFO      
            
                    CORE_EXPECTS_EQ(info_1.m_dbg_str_pattern, "g(1, 2, x...)");

                    const char * exp = R"(Arguments: 2, Inf
Arg[0]: [1, 1], Remaining: 1, Inf
Arg[1]: [1, 1], Remaining: 0, Inf
Arg[2]: [0, Inf], Remaining: 0, 0)";

                    CORE_EXPECTS_EQ(info_1.m_dbg_labels, exp);
            #endif
            } 

            /////////////////////////////////////////////////

            {
                const m2o_pattern::PatternInfo info_2 = m2o_pattern::BuildPatternInfo(
                    "g(1 a... 10 11 b...)");

                CORE_EXPECTS_EQ(info_2.m_arguments_info[0].m_cardinality, (Range{ 1, 1 }));
                CORE_EXPECTS_EQ(info_2.m_arguments_info[1].m_cardinality, (Range{ 0, infinity }));
                CORE_EXPECTS_EQ(info_2.m_arguments_info[2].m_cardinality, (Range{ 1, 1 }));
                CORE_EXPECTS_EQ(info_2.m_arguments_info[3].m_cardinality, (Range{ 1, 1 }));
                CORE_EXPECTS_EQ(info_2.m_arguments_info[4].m_cardinality, (Range{ 0, infinity }));

                CORE_EXPECTS_EQ(info_2.m_arguments_info[0].m_remaining, (Range{ 2, infinity }));
                CORE_EXPECTS_EQ(info_2.m_arguments_info[1].m_remaining, (Range{ 2, infinity }));
                CORE_EXPECTS_EQ(info_2.m_arguments_info[2].m_remaining, (Range{ 1, infinity }));
                CORE_EXPECTS_EQ(info_2.m_arguments_info[3].m_remaining, (Range{ 0, infinity }));
                CORE_EXPECTS_EQ(info_2.m_arguments_info[4].m_remaining, (Range{ 0, 0 }));
            }

                    /////////////////////////////////////////////////

            {
                const m2o_pattern::PatternInfo info_3 = m2o_pattern::BuildPatternInfo(
                    "g(1 a.. 10 11 b...)");

                CORE_EXPECTS_EQ(info_3.m_arguments_info[0].m_cardinality, (Range{ 1, 1 }));
                CORE_EXPECTS_EQ(info_3.m_arguments_info[1].m_cardinality, (Range{ 1, infinity }));
                CORE_EXPECTS_EQ(info_3.m_arguments_info[2].m_cardinality, (Range{ 1, 1 }));
                CORE_EXPECTS_EQ(info_3.m_arguments_info[3].m_cardinality, (Range{ 1, 1 }));
                CORE_EXPECTS_EQ(info_3.m_arguments_info[4].m_cardinality, (Range{ 0, infinity }));

                CORE_EXPECTS_EQ(info_3.m_arguments_info[0].m_remaining, (Range{ 3, infinity }));
                CORE_EXPECTS_EQ(info_3.m_arguments_info[1].m_remaining, (Range{ 2, infinity }));
                CORE_EXPECTS_EQ(info_3.m_arguments_info[2].m_remaining, (Range{ 1, infinity }));
                CORE_EXPECTS_EQ(info_3.m_arguments_info[3].m_remaining, (Range{ 0, infinity }));
                CORE_EXPECTS_EQ(info_3.m_arguments_info[4].m_remaining, (Range{ 0, 0 }));
            }

            /////////////////////////////////////////////////

            {
                const std::string expr = "g(1 2 x... y... z... 3 4)";
                const m2o_pattern::PatternInfo info_4 = m2o_pattern::BuildPatternInfo(expr.c_str());
                #if DJUP_DEBUG_PATTERN_INFO
                    //Print(expr, "\n", info_4.m_dbg_labels, "\n");
                #endif
            }

                        /////////////////////////////////////////////////

            {
                const std::string expr = "g(1 2 (real x real y)... 3 4)";
                const m2o_pattern::PatternInfo info_5 = m2o_pattern::BuildPatternInfo(expr.c_str());
                #if DJUP_DEBUG_PATTERN_INFO
                    //Print(expr, "\n", info_5.m_dbg_labels, "\n");
                #endif
            }

            {
                PrintLn();
                const Tensor pattern = "g(1 a... 10 11 b... 12 13 c... 14)"_t;
                const uint32_t target_size = 10;
                const auto & arguments = pattern.GetExpression()->GetArguments();
                const m2o_pattern::PatternInfo info_6 = m2o_pattern::BuildPatternInfo(pattern);
                std::vector<Range> usable;
                usable.resize(info_6.m_arguments_info.size());
                for (uint32_t i = 0; i < info_6.m_arguments_info.size(); ++i)
                {
                    if (info_6.m_arguments_info[i].m_cardinality.m_min !=
                        info_6.m_arguments_info[i].m_cardinality.m_max)
                    {
                        usable[i] = GetUsableCount(info_6.m_arguments_info[i], target_size - i, 1);
                        PrintLn(ToSimplifiedString(arguments[i]), ", usable: [", usable[i], "]");
                    }
                }
            }

            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup
