
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <private/common.h>
#include <vector>
#include <optional>
#include <core/graph_wiz.h>
#include <core/hash.h>
#include <private/name.h>
#include <djup/tensor.h>

namespace djup
{
    class Tensor;

    struct ExpressionMetadata
    {
        Tensor m_type;
        bool m_is_constant = false;
    };

    class Expression
    {
    public:

        Expression();

        Expression(Name i_name, Span<const Tensor> i_arguments, std::optional<ExpressionMetadata> i_metadata = {});

        const Name & GetName() const { return m_name; }

        Hash GetHash() const { return m_hash; }

        const Tensor & GetArgument(size_t i_index) const { return m_arguments[i_index]; }

        const std::vector<Tensor> & GetArguments() const { return m_arguments; }

        const std::optional<ExpressionMetadata> & GetMetadata() const { return m_metadata; }

        bool IsConstant() const { return m_metadata.has_value() && m_metadata->m_is_constant; }

        const Tensor & GetType() const { return m_metadata ? m_metadata->m_type : EmptyTensor(); }

    private:
        Hash m_hash;
        Name m_name;
        std::vector<Tensor> m_arguments;
        std::optional<ExpressionMetadata> m_metadata;
        #if DJUP_DEBUG_STRING
            std::string m_debug_string;
        #endif
    };

    inline Hash & operator << (Hash & i_dest, const Expression & i_src)
    {
        return i_dest << i_src.GetHash();
    }

    Hash & operator << (Hash & i_dest, const Tensor & i_src);

    [[nodiscard]] Tensor MakeExpression(Name i_name, Span<const Tensor> i_arguments = {}, std::optional<ExpressionMetadata> i_metadata = {});

    [[nodiscard]] Tensor MakeExpression(Expression && i_source);

    [[nodiscard]] Tensor TensorType(Name i_scalar_type, Tensor i_shape_vector);

    [[nodiscard]] const Name & GetIdentifierName(const Tensor & i_identifier);
    
    [[nodiscard]] Tensor MakeLiteral(bool i_bool_value);

    [[nodiscard]] Tensor MakeLiteral(int64_t i_integer_value);

    [[nodiscard]] bool AlwaysEqual(const Expression & i_first, const Expression & i_second);

    bool NameIs(const Tensor & i_tensor, const Name & i_name);

    bool NameIs(const Tensor & i_tensor, const ConstexprName & i_name);

    void ToSimplifiedStringForm(StringBuilder & i_dest, const Tensor & i_source, 
        size_t i_depth = std::numeric_limits<size_t>::max(), bool tidy = true);

    std::string ToSimplifiedStringForm(const Expression& i_source,
        size_t i_depth = std::numeric_limits<size_t>::max(), bool tidy = true);

    enum class FunctionFlags : uint16_t
    {
        None = 0,
        Associative = 1 << 0,
        Commutative = 1 << 1,
    };

    FunctionFlags GetFunctionFlags(const Name & i_function_name);

    GraphWizGraph ExpressionToGraph(const Expression& i_source,
        size_t i_depth = std::numeric_limits<int32_t>::max(), bool i_tidy = true);

    GraphWizGraph TensorToGraph(const Tensor& i_source,
        size_t i_depth = std::numeric_limits<int32_t>::max(), bool i_tidy = true);

    template <auto VALUE>
        [[nodiscard]] const Tensor & MakeLiteral()
    {
        if constexpr(std::is_same_v<decltype(VALUE), bool>)
        {
            static const Tensor s_value = MakeLiteral(VALUE);
            return s_value;
        }
        else
        {
            static_assert(std::is_integral_v<decltype(VALUE)>);
            static const Tensor s_value = MakeLiteral(NumericCast<int64_t>(VALUE));
            return s_value;
        }
    }
}
