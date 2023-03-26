
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace core
{
    template <typename CONTAINER, typename PREDICATE>
        bool AllOf(const CONTAINER & i_container, PREDICATE i_predicate)
    {
        for(const auto & elem : i_container)
            if(!i_predicate(elem))
                return false;
        return true;
    }

    template <typename CONTAINER, typename PREDICATE>
        bool AnyOf(const CONTAINER & i_container, PREDICATE i_predicate)
    {
        for(const auto & elem : i_container)
            if(i_predicate(elem))
                return true;
        return false;
    }

    template <typename CONTAINER, typename ELEMENT>
        bool Contains(const CONTAINER & i_container, const ELEMENT & i_target)
    {
        for(const auto & element : i_container)
            if(element == i_target)
                return true;
        return false;
    }

    template <typename TYPE_1, typename TYPE_2>
        auto Min(const TYPE_1& i_first, const TYPE_2& i_second) noexcept
    {
        return i_first < i_second ? i_first : i_second;
    }

    template <typename TYPE_1, typename TYPE_2>
        auto Max(const TYPE_1& i_first, const TYPE_2& i_second) noexcept
    {
        return i_first > i_second ? i_first : i_second;
    }
}
