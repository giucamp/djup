
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <private/common.h>
#include <private/expression.h>
#include <private/namespace.h>
#include <private/builtin_names.h>
#include <core/algorithms.h>
#include <core/to_chars.h>
#include <djup/tensor.h>
#include <core/algorithms.h>
#include <core/bits.h>
#include <core/hash.h>
#include <core/flags.h>

namespace djup
{
    Expression::Expression()
    {
        m_hash << m_name;
        m_hash << m_arguments;
    }

    Expression::Expression(Name i_name, Span<const Tensor> i_arguments, std::optional<ExpressionMetadata> i_metadata)
        : m_name(std::move(i_name)),
          m_arguments(i_arguments.begin(), i_arguments.end()), 
          m_metadata(std::move(i_metadata))
    {
        static const Name non_constants[] = {builtin_names::Identifier, builtin_names::RepetitionsZeroToMany, 
            builtin_names::RepetitionsOneToMany, builtin_names::RepetitionsZeroToOne, builtin_names::AssociativeIdentifier};

        if(!Contains(non_constants, m_name)
            && AllOf(m_arguments, djup::IsConstant))
        {
            if(!m_metadata)
                m_metadata = ExpressionMetadata{};
            m_metadata->m_is_constant = true;
        }

        if(HasFlag(GetFunctionFlags(m_name), FunctionFlags::Commutative))
        {
            std::sort(m_arguments.begin(), m_arguments.end(), 
                [](const Tensor & i_first, const Tensor & i_second){
                    return i_first.GetExpression()->GetHash() < i_second.GetExpression()->GetHash(); });
        }
        m_hash << m_name;
        m_hash << m_arguments;

        // constants have always lower hash than non-constants
        auto hash = m_hash.GetValue();
        if(IsConstant())
            hash &= ~bit_reverse<decltype(hash)>(0);
        else
            hash |= bit_reverse<decltype(hash)>(0);

        m_hash = HashFromValue(hash);

        #if DJUP_DEBUG_STRING
            m_debug_string = ToSimplifiedStringForm(*this);
        #endif
    }

    Hash & operator << (Hash & i_dest, const Tensor & i_src)
    {
        return i_dest << i_src.GetExpression()->GetHash();
    }

    Tensor MakeExpression(Name i_name, Span<const Tensor> i_arguments, 
        std::optional<ExpressionMetadata> i_metadata)
    {
        return {std::make_shared<Expression>(std::move(i_name), i_arguments, std::move(i_metadata))};
    }

    Tensor MakeExpression(Expression && i_source)
    {
        return {std::make_shared<Expression>(std::move(i_source))};
    }

    Tensor MakeLiteral(bool i_bool_value)
    {
        char buffer[8];
        Name name = ToCharsView(buffer, i_bool_value);
        ExpressionMetadata metadata = { TensorType(builtin_names::Bool, {}) };
        metadata.m_is_constant = true;
        return MakeExpression(builtin_names::Literal, { MakeExpression(std::move(name), {}) }, std::move(metadata));
    }

    Tensor MakeLiteral(int64_t i_integer_value)
    {
        char buffer[std::numeric_limits<int64_t>::max_digits10 + 4];
        Name name = ToCharsView(buffer, i_integer_value);
        ExpressionMetadata metadata = { TensorType(builtin_names::Int, {}) };
        metadata.m_is_constant = true;
        return MakeExpression(builtin_names::Literal, { MakeExpression(std::move(name), {}) }, std::move(metadata));
    }

    Tensor TensorType(Name i_scalar_type, Tensor i_shape_vector)
    {
        Tensor res = MakeExpression(builtin_names::TensorType,
            { MakeExpression(i_scalar_type), std::move(i_shape_vector) });
        return res;
    }

    bool NameIs(const Tensor & i_tensor, const Name & i_name)
    {
        return i_tensor.GetExpression()->GetName() == i_name;
    }

    bool NameIs(const Tensor & i_tensor, const ConstexprName & i_name)
    {
        return i_tensor.GetExpression()->GetName() == i_name;
    }

    bool AlwaysEqual(const Expression & i_first, const Expression & i_second)
    {
        if(i_first.GetHash() != i_second.GetHash())
            return false;

        if(i_first.GetName() != i_second.GetName())
            return false;

        const size_t argument_count = i_first.GetArguments().size();
        if(argument_count != i_second.GetArguments().size())
            return false;

        for(size_t argument_index = 0; argument_index < argument_count; argument_index++)
            if(!AlwaysEqual(i_first.GetArgument(argument_index), i_second.GetArgument(argument_index)))
                return false;

        return true;
    }
}
