
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <tests/test_utils.h>
#include <private/m2o_pattern/m2o_substitutions_builder.h>

namespace djup
{
    namespace tests
    {
        void TestSubstitutionBuilder()
        {
            Print("Test: djup - Substitution Builder...");

            using namespace m2o_pattern;

            // test 1
            {
                SubstitutionsBuilder builder;

                builder.Open(2);

                builder.Add({ { "x", "1"_t } });
                builder.Add({ { "x", "2"_t } });
                builder.Add({ { "x", "3"_t } });

                builder.Close(1);

                builder.Open(1);

                builder.Add({ { "x", "4"_t } });
                builder.Add({ { "x", "5"_t } });
                builder.Add({ { "x", "6"_t } });

                builder.Close(2);

                CORE_EXPECTS(builder.GetSubstitutions().size() == 1);

                DJUP_EXPR_EXPECTS_EQ(
                    ToSimplifiedString(builder.GetSubstitutions().at(0).m_value),
                    "Tuple(Tuple(1, 2, 3), Tuple(4, 5, 6))");
                
            }

            // test 2
            {
                SubstitutionsBuilder builder;

                builder.Open(3);

                builder.Add({ { "x", "1"_t } });
                builder.Add({ { "x", "2"_t } });
                builder.Add({ { "x", "3"_t } });

                builder.Close(1);

                builder.Open(1);

                builder.Add({ { "x", "4"_t } });
                builder.Add({ { "x", "5"_t } });
                builder.Add({ { "x", "6"_t } });

                builder.Close(2);

                builder.Open(2);

                builder.Add({ { "x", "7"_t } });
                builder.Add({ { "x", "8"_t } });
                builder.Add({ { "x", "9"_t } });

                builder.Close(2);

                builder.Open(2);

                builder.Add({ { "x", "10"_t } });
                builder.Add({ { "x", "11"_t } });
                builder.Add({ { "x", "12"_t } });

                builder.Close(3);

                CORE_EXPECTS(builder.GetSubstitutions().size() == 1);               

                DJUP_EXPR_EXPECTS_EQ(
                    ToSimplifiedString(builder.GetSubstitutions().at(0).m_value),
                    "Tuple(Tuple(Tuple(1, 2, 3), Tuple(4, 5, 6)), "
                    "Tuple(Tuple(7, 8, 9)), Tuple(Tuple(10, 11, 12)))");
            }

            // test 3
            {
                SubstitutionsBuilder builder;

                builder.Open(2);

                builder.Add({ { "x", "1"_t } });
                builder.Add({ { "y", "10"_t } });
                builder.Add({ { "x", "2"_t } });
                builder.Add({ { "y", "11"_t } });
                builder.Add({ { "x", "3"_t } });
                builder.Add({ { "y", "12"_t } });

                builder.Close(1);

                builder.Open(1);

                builder.Add({ { "x", "4"_t } });
                builder.Add({ { "y", "13"_t } });
                builder.Add({ { "x", "5"_t } });
                builder.Add({ { "y", "14"_t } });
                builder.Add({ { "x", "6"_t } });
                builder.Add({ { "y", "15"_t } });

                builder.Close(2);

                CORE_EXPECTS(builder.GetSubstitutions().size() == 2);

                DJUP_EXPR_EXPECTS_EQ(
                    ToSimplifiedString(builder.GetSubstitutions().at(0).m_value),
                    "Tuple(Tuple(1, 2, 3), Tuple(4, 5, 6))");

                DJUP_EXPR_EXPECTS_EQ(
                    ToSimplifiedString(builder.GetSubstitutions().at(1).m_value),
                    "Tuple(Tuple(10, 11, 12), Tuple(13, 14, 15))");
            }

            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup
