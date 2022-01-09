
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <core/diagnostic.h>
#include <djup/tensor.h>

namespace djup
{
    inline void ExprExpectsEq( std::string_view i_first, std::string_view i_second,
        const char * i_source_file, int i_source_line)
    {
        Tensor first(i_first), second(i_second);

        if(!(AlwaysEqual(first, second)))
            Error(i_source_file, "(", i_source_line, ") - DJUP_EXPECTS_EQ - first is:\n", ToSimplifiedStringForm(first),
                ", \nsecond is:\n", ToSimplifiedStringForm(second) );
    }

    #define DJUP_EXPR_EXPECTS_EQ(first, second) ::djup::ExprExpectsEq(first, second, __FILE__, __LINE__)
}
