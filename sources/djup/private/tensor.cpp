
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <djup/tensor.h>
#include <private/expression.h>

namespace djup
{
    Tensor::Tensor(ScalarConst, double i_scalar)
        : m_expression(std::make_shared<Expression>(i_scalar))
    {

    }

    Tensor::Tensor(ScalarConst, int64_t i_scalar)
        : m_expression(std::make_shared<Expression>(i_scalar))
    {

    }

    Tensor::Tensor(ScalarConst, bool i_scalar)
        : m_expression(std::make_shared<Expression>(i_scalar))
    {

    }
}
