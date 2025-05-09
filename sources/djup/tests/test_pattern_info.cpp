
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <private/pattern/pattern_info.h>
#include <tests/test_utils.h>

namespace djup
{
    namespace tests
    {
        void PatternInfo()
        {
            Print("Test: djup - Pattern Info...");

            using namespace pattern;

            const auto infinity = std::numeric_limits<int32_t>::max();

            {
                const pattern::PatternInfo info_1 = pattern::BuildPatternInfo("g(1 2 x...)");

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
                const pattern::PatternInfo info_2 = pattern::BuildPatternInfo(
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
                const pattern::PatternInfo info_3 = pattern::BuildPatternInfo(
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
                const pattern::PatternInfo info_4 = pattern::BuildPatternInfo(expr.c_str());
                #if DJUP_DEBUG_PATTERN_INFO
                    //Print(expr, "\n", info_4.m_dbg_labels, "\n");
                #endif
            }
                        /////////////////////////////////////////////////

            {
                const std::string expr = "g(1 2 (real x real y)... 3 4)";
                const pattern::PatternInfo info_5 = pattern::BuildPatternInfo(expr.c_str());
                #if DJUP_DEBUG_PATTERN_INFO
                    //Print(expr, "\n", info_5.m_dbg_labels, "\n");
                #endif
            }
            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup
