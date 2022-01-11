
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/scalar_type.h>
#include <core/diagnostic.h>

namespace djup
{
    ScalarType GetSmallestCommonSuperset(Span<const ScalarType> i_elements)
    {
        if(i_elements.empty())
            return ScalarType::Any;

        using ScalarTypeInt = std::underlying_type_t<ScalarType>;
        ScalarTypeInt common_int = 0;
        for(auto element : i_elements)
        {
            if(element == ScalarType::Any)
                return ScalarType::Any;

            const auto common = static_cast<ScalarType>(common_int);
            if(!IsSupersetOf(common, element) && !IsSupersetOf(element, common))
                return ScalarType::Any;

            common_int |= static_cast<ScalarTypeInt>(element);
        }
        return static_cast<ScalarType>(common_int);
    }

} // namespace djup
