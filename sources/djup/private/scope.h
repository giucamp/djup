
#pragma once
#include <memory>
#include <private/discrimination_network.h>

namespace djup
{
    class Scope
    {
    public:
        
        Scope(const std::shared_ptr<const Scope> & i_parent);

        // disable copy
        Scope(const Scope &) = delete;
        Scope & operator = (const Scope &) = delete;

        /* Returns the root scope, which is always empty and unmodifiable */
        static const std::shared_ptr<const Scope> & Root();

        std::shared_ptr<const Scope> const & GetParent() const { return m_parent; }

    private:
        std::shared_ptr<const Scope> const m_parent;
    };
}
