
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <vector>
#include <optional>
#include <core/graph_wiz.h>
#include <core/hash.h>
#include <djup/name.h>
#include <djup/tensor.h>

namespace djup
{
    class Tensor;

    struct ExpressionMetadata
    {
        Tensor m_type;
        const char* m_source_file{}; // to do: SharedImmutableVector needed here
        uint32_t m_source_line{std::numeric_limits<uint32_t>::max()};
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

    [[nodiscard]] const Name& GetIdentifierName(const Tensor& i_identifier);

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

    FunctionFlags GetFunctionFlags(const Name & i_function_name);

    GraphWizGraph TensorToGraph(const Tensor& i_source,
        FormatFlags i_format_flags = FormatFlags::Tidy,
        size_t i_depth = std::numeric_limits<int32_t>::max());

}
