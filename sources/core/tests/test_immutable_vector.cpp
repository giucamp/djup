
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <core/immutable_vector.h>
#include <core/diagnostic.h>

namespace core
{
    namespace tests
    {
        void ImmutableVector_()
        {
            Print("Test: Core - ImmutableVector...");

            ImmutableVector<int> int_vect;

            {
                std::vector v = { 1, 2, 3 };
                ImmutableVector<int> iv(v.begin(), v.end());
                CORE_EXPECTS_EQ(iv.size(), 3u);
                CORE_EXPECTS_EQ(iv[0], 1);
                CORE_EXPECTS_EQ(iv[1], 2);
                CORE_EXPECTS_EQ(iv[2], 3);

                ImmutableVector<int> iv1(iv);
                CORE_EXPECTS_EQ(iv1.size(), 3u);
                CORE_EXPECTS_EQ(iv1[0], 1);
                CORE_EXPECTS_EQ(iv1[1], 2);
                CORE_EXPECTS_EQ(iv1[2], 3);
                
                ImmutableVector<int> iv2(v.data(), v.data() + v.size());
                CORE_EXPECTS_EQ(iv2.size(), 3u);
                CORE_EXPECTS_EQ(iv2[0], 1);
                CORE_EXPECTS_EQ(iv2[1], 2);
                CORE_EXPECTS_EQ(iv2[2], 3);

                int i = 1;
                for (auto& el : iv2)
                {
                    CORE_EXPECTS_EQ(el, i);
                    i++;
                }

                ImmutableVector<int> iv3(iv2.begin(), iv2.end());
                CORE_EXPECTS_EQ(iv3.size(), 3u);
                CORE_EXPECTS_EQ(iv3[0], 1);
                CORE_EXPECTS_EQ(iv3[1], 2);
                CORE_EXPECTS_EQ(iv3[2], 3);
            }

            PrintLn("successful");
        }

    } // namespace tests

} // namespace core
