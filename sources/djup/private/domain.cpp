
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/domain.h>
#include <core/diagnostic.h>

namespace djup
{
    Domain GetSmallestCommonSuperset(Span<const Domain> i_elements)
    {
        if(i_elements.empty())
            return Domain::Any;

        using DomainInt = std::underlying_type_t<Domain>;
        DomainInt common_int = 0;
        for(auto element : i_elements)
        {
            if(element == Domain::Any)
                return Domain::Any;

            const auto common = static_cast<Domain>(common_int);
            if(!IsSupersetOf(common, element) && !IsSupersetOf(element, common))
                return Domain::Any;

            common_int |= static_cast<DomainInt>(element);
        }
        return static_cast<Domain>(common_int);
    }

} // namespace djup
