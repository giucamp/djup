
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <core/diagnostic.h>

namespace core
{
    namespace tests
    {
        void Core();
    }
}

namespace djup
{
    namespace tests
    {
        void Djup();
    }
}

int main()
{
    core::tests::Core();
    djup::tests::Djup();
    system("PAUSE");
}
