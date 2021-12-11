
#pragma once
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
