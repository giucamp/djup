
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/scope.h>

namespace djup
{
    const std::shared_ptr<const Scope> & Scope::Root()
    {
        static const std::shared_ptr<const Scope> root = std::make_shared<Scope>(nullptr);
        return root;
    }

    Scope::Scope(const std::shared_ptr<const Scope> & i_parent)
        : m_parent(i_parent)
    {
    }
}
