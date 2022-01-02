
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <memory>
#include <private/discrimination_network.h>
#include <private/type.h>

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

        void AddSubstitutionAxiom(const Tensor & i_what, const Tensor & i_with, const Tensor & i_when);

        void AddTypeAxiom(const Tensor & i_what, const Tensor & i_type, const Tensor & i_when);

    private:
        std::shared_ptr<const Scope> const m_parent;
    };

    Tensor MakeScope(Span<Tensor const> i_arguments);
}
