
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
    // defined in TensorToString.cpp
    void ToSimplifiedString(StringBuilder & i_dest, const Expression& i_source,
        FormatFlags i_format_flags, size_t i_depth);

    FunctionFlags GetFunctionFlags(const Expression & i_expression)
    {
        const Name & name = i_expression.GetName();

        FunctionFlags flags = {};
        if (name == "Add" || name == "Mul" || name == "Equals")
            flags = CombineFlags(flags, FunctionFlags::Commutative);
        if (name == "Add" || name == "Mul" || name == "MatMul")
            flags = CombineFlags(flags, FunctionFlags::Associative);
        return flags;
    }

    ExpressionKind GetExpressionKind(const Expression & i_expression)
    {
        if (i_expression.GetMetadata().m_is_constant)
            return ExpressionKind::Constant;
        else if (i_expression.GetMetadata().m_is_identifier)
            return ExpressionKind::Identifier;        
        else if (i_expression.GetMetadata().m_is_repetition)
            return ExpressionKind::Variadic;
        else
            return ExpressionKind::VariableFunction;
    }

    IntInterval GetCardinality(const Tensor & i_expression)
    {
        if (NameIs(i_expression, builtin_names::RepetitionsZeroToMany))
            return { 0, IntInterval::s_infinite };
        else if (NameIs(i_expression, builtin_names::RepetitionsZeroToOne))
            return { 0, 1 };
        else if (NameIs(i_expression, builtin_names::RepetitionsOneToMany) || NameIs(i_expression, builtin_names::AssociativeIdentifier))
            return { 1, IntInterval::s_infinite };
        else
            return { 1, 1 };
    }

    bool IsRepetition(const Tensor & i_expression)
    {
        return NameIs(i_expression, builtin_names::RepetitionsZeroToMany)
            || NameIs(i_expression, builtin_names::RepetitionsZeroToOne)
            || NameIs(i_expression, builtin_names::RepetitionsOneToMany);
    }

    Expression::Expression()
    {
        m_hash << m_name;
        m_hash << m_arguments;
    }

    Expression::Expression(TensorType i_type, Name i_name,
            Span<const Tensor> i_arguments, ExpressionMetadata i_metadata)
        : m_name(std::move(i_name)),
          m_type(std::move(i_type)),
          m_arguments(i_arguments.begin(), i_arguments.end()),
          m_metadata(std::move(i_metadata))
    {
        static const Name non_constants[] = {builtin_names::RepetitionsZeroToMany, 
            builtin_names::RepetitionsOneToMany, builtin_names::RepetitionsZeroToOne, builtin_names::AssociativeIdentifier};

        if(!Contains(non_constants, m_name) && !m_metadata.m_is_identifier
            && AllOf(m_arguments, djup::IsConstant))
        {
            m_metadata.m_is_constant = true;
        }

        m_metadata.m_is_repetition = m_name == builtin_names::RepetitionsZeroToMany ||
            m_name == builtin_names::RepetitionsZeroToOne ||
            m_name == builtin_names::RepetitionsOneToMany;

        if(HasFlag(GetFunctionFlags(*this), FunctionFlags::Commutative))
        {
            std::sort(m_arguments.begin(), m_arguments.end(), 
                [](const Tensor & i_first, const Tensor & i_second){
                    return i_first.GetExpression()->GetHash() < i_second.GetExpression()->GetHash(); });
        }
        m_hash << m_name;
        m_hash << m_arguments;

        /* constants have always lower hash than non-constants, so that
           they appear first after sorting commutative function arguments
           and parameters. */
        auto hash = m_hash.GetValue();
        if(m_metadata.m_is_constant)
            hash &= ~bit_reverse<decltype(hash)>(0);
        else
            hash |= bit_reverse<decltype(hash)>(0);
        m_hash = HashFromValue(hash);

        #if DJUP_DEBUG_STRING
            StringBuilder dest;
            ToSimplifiedString(dest, *this, FormatFlags::Tidy, std::numeric_limits<size_t>::max());
            m_debug_string = dest.StealString();
        #endif
    }

    Hash & operator << (Hash & i_dest, const Tensor & i_src)
    {
        return i_dest << i_src.GetExpression()->GetHash();
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
