
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <vector>
#include <core/graph_wiz.h>
#include <core/hash.h>
#include <core/immutable_vector.h>
#include <core/name.h>
#include <djup/tensor.h>
#include <private/tensor_type.h>

namespace djup
{
    class Tensor;

    struct ExpressionMetadata
    {
        ImmutableVector<char> m_source_file;
        uint64_t m_source_location{std::numeric_limits<uint32_t>::max()};
        bool m_is_constant = false;
        bool m_is_literal = false;
        bool m_is_identifier = false;
        bool m_is_repetition = false;
    };

    class Expression
    {
    public:

        Expression();

        Expression(TensorType i_type, Name i_name, 
            Span<const Tensor> i_arguments, ExpressionMetadata i_metadata);

        const Name & GetName() const { return m_name; }

        Hash GetHash() const { return m_hash; }

        const Tensor & GetArgument(size_t i_index) const { return m_arguments[i_index]; }

        const std::vector<Tensor> & GetArguments() const { return m_arguments; }

        const ExpressionMetadata & GetMetadata() const { return m_metadata; }

        const TensorType & GetType() const { return m_type; }

    private:
        Hash m_hash;
        Name m_name;
        TensorType m_type;
        std::vector<Tensor> m_arguments;
        ExpressionMetadata m_metadata;
        #if DJUP_DEBUG_STRING
            std::string m_debug_string;
        #endif
    };

    inline Hash & operator << (Hash & i_dest, const Expression & i_src)
    {
        return i_dest << i_src.GetHash();
    }

    Hash & operator << (Hash & i_dest, const Tensor & i_src);

    [[nodiscard]] bool AlwaysEqual(const Expression & i_first, const Expression & i_second);

    bool NameIs(const Tensor & i_tensor, const Name & i_name);

    bool NameIs(const Tensor & i_tensor, const ConstexprName & i_name);

    void ToSimplifiedString(StringBuilder & i_dest, const Tensor & i_source, 
        FormatFlags i_format_flags = FormatFlags::Tidy, size_t i_depth = std::numeric_limits<size_t>::max());

    enum class FunctionFlags : uint16_t
    {
        None = 0,
        Associative = 1 << 0,
        Commutative = 1 << 1,
    };

    FunctionFlags GetFunctionFlags(const Expression & i_expression);

    enum class ExpressionKind : uint16_t
    {
        Constant,
        Identifier,
        VariableFunction,
        Variadic
    };

    ExpressionKind GetExpressionKind(const Expression & i_expression);

    GraphWizGraph TensorToGraph(const Tensor& i_source,
        FormatFlags i_format_flags = FormatFlags::Tidy,
        size_t i_depth = std::numeric_limits<int32_t>::max());

}
