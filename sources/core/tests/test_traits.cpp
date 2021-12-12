
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <core/traits.h>
#include <core/diagnostic.h>
#include <vector>
#include <deque>
#include <list>

namespace djup
{
    namespace tests
    {
        void Traits()
        {
            Print("Test: Core - Traits...");

            using namespace std;

            static_assert(IsContigousContainerV<vector<int>>);
            static_assert(!IsContigousContainerV<deque<int>>);
            static_assert(!IsContigousContainerV<list<int>>);
            static_assert(IsContigousContainerOfV<vector<int>, int>);
            static_assert(!IsContigousContainerOfV<vector<int>, float>);
            static_assert(!IsContigousContainerOfV<deque<int>, float>);

            static_assert(IsContainerV<vector<int>>);
            static_assert(IsContainerV<deque<int>>);
            static_assert(IsContainerV<list<int>>);

            static_assert(IsContainerOfV<vector<int>, int>);
            static_assert(IsContainerOfV<deque<int>, int>);
            static_assert(IsContainerOfV<list<int>, int>);
            static_assert(!IsContainerOfV<vector<int>, float>);
            static_assert(!IsContainerOfV<deque<int>, float>);
            static_assert(!IsContainerOfV<list<int>, float>);

            PrintLn("successful");
        }

    } // namespace tests

} // namespace djup
