
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/pattern/pattern_info.h>
#include <core/diagnostic.h>

namespace djup
{
    namespace tests
    {
        void PatternInfo()
        {
            Print("Test: djup - Pattern Info...");

            using namespace pattern;

                            // info_1

            const pattern::PatternInfo info_1 = pattern::BuildPatternInfo("g(1 2 x...)");

            auto infinity = std::numeric_limits<int32_t>::max();

            CORE_EXPECTS(info_1.m_flags == FunctionFlags{});
            CORE_EXPECTS_EQ(info_1.m_labels_range, (Range{2, infinity }) );

            CORE_EXPECTS_EQ(NumericCast<int32_t>(info_1.m_labels_info.size()), 3);

            CORE_EXPECTS_EQ(info_1.m_labels_info[0].m_cardinality, (Range{ 1, 1 }) );
            CORE_EXPECTS_EQ(info_1.m_labels_info[1].m_cardinality, (Range{ 1, 1 }) );
            CORE_EXPECTS_EQ(info_1.m_labels_info[2].m_cardinality, (Range{ 0, infinity }) );
            CORE_EXPECTS_EQ(info_1.m_labels_info[0].m_remaining, (Range{ 1, infinity }));
            CORE_EXPECTS_EQ(info_1.m_labels_info[1].m_remaining, (Range{ 0, infinity }));
            CORE_EXPECTS_EQ(info_1.m_labels_info[2].m_remaining, (Range{ 0, 0 }));

            #if !defined(DJUP_DEBUG_PATTERN_INFO)
            #error DJUP_DEBUG_PATTERN_INFO must be defined
            #endif
            #if DJUP_DEBUG_PATTERN_INFO           
            
                CORE_EXPECTS_EQ(info_1.m_dbg_str_pattern, "g(1, 2, x...)");

                const char * exp = R"(Arguments: 2, Inf
Arg[0]: 1, 1 Remaining: 1, Inf
Arg[1]: 1, 1 Remaining: 0, Inf
Arg[2]: 0, Inf Remaining: 0, 0)";

                CORE_EXPECTS_EQ(info_1.m_dbg_labels, exp);

            #endif

            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup
