
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <djup/tensor.h>
#include <private/expression.h>
#include <private/scope.h>
#include <private/builtin_names.h>

namespace djup
{
    namespace
    {
        bool IsAny(const Tensor & i_tensor)
        {
            return i_tensor.GetExpression()->GetArguments().empty() &&
                i_tensor.GetExpression()->GetName() == builtin_names::Any;
        }
    }

    bool Is(const Tensor & i_tensor, const Tensor & i_pattern)
    {
        if(i_tensor.IsEmpty() || i_pattern.IsEmpty())
            return false;

        if(IsAny(i_pattern))
            return true;

        const Tensor & target_type = i_tensor.GetExpression()->GetType();
        if(target_type.IsEmpty())
            return false;

        if(target_type.GetExpression()->GetName() == builtin_names::TensorType &&
            i_pattern.GetExpression()->GetName() == builtin_names::TensorType)
        {
            const Name & target_scalar = target_type.GetExpression()->GetArgument(0).GetExpression()->GetName();
            const Name & pattern_scalar = i_pattern.GetExpression()->GetArgument(0).GetExpression()->GetName();

            if(!GetActiveScope()->ScalarTypeBelongsTo(target_scalar, pattern_scalar))
                return false;

            const Tensor & target_shape = target_type.GetExpression()->GetArgument(1);
            const Tensor & pattern_shape = i_pattern.GetExpression()->GetArgument(1);
            if(!AlwaysEqual(target_shape, pattern_shape))
                return false;

            return true;
        }

        return false;
    }
}
