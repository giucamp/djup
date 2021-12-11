
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
