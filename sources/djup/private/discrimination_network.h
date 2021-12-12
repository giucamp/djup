
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <memory>
#include <unordered_map>
#include <private/name.h>
#include <private/expression.h>

namespace djup
{
    class DiscriminationNetwork
    {
    public:



    private:

        struct Node
        {
            std::unordered_map<Name, std::shared_ptr<Node>> m_map;
        };

        Node m_root;
    };
}
