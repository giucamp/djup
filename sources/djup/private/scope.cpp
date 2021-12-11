
#pragma once
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
