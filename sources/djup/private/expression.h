
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <vector>
#include <limits>
#include <type_traits>
#include <core/hash.h>
#include <core/numeric_cast.h>
#include <core/to_string.h>
#include <private/name.h>
#include <djup/tensor.h>

namespace djup
{
    class Tensor;
    class TensorType;

    struct ExpressionData
    {
        Name m_name;
        std::vector<Tensor> m_arguments;
        Tensor m_type;

        union
        {
            struct
            {
                bool m_is_constant;
            };
            uint32_t m_all_flags = 0;
        };
    };

    class Expression
    {
    public:

        Expression() = default;

        Expression(ExpressionData && i_data);

        const Name & GetName() const { return m_data.m_name; }

        Hash GetHash() const { return m_hash; }

        bool IsConstant() const { return m_data.m_is_constant; }

        auto GetAllFlags() const { return m_data.m_all_flags; }

        const Tensor & GetArgument(size_t i_index) const { return m_data.m_arguments[i_index]; }

        const std::vector<Tensor> & GetArguments() const { return m_data.m_arguments; }

        const Tensor & GetType() const { return m_data.m_type; }

        const ExpressionData & GetExpressionData() const { return m_data; }

    private:
        Hash m_hash;
        ExpressionData m_data;
    };

    inline Hash & operator << (Hash & i_dest, const Expression & i_src)
    {
        return i_dest << i_src.GetHash();
    }

    Hash & operator << (Hash & i_dest, const Tensor & i_src);

    [[nodiscard]] Tensor MakeExpression(ExpressionData && i_data);
   
    [[nodiscard]] Tensor MakeExpression(Name i_name, Span<const Tensor> i_arguments = {});

    [[nodiscard]] Tensor MakeConstantExpression(Name i_name, Span<const Tensor> i_arguments = {});

    [[nodiscard]] Tensor MakeLiteral(bool i_bool_value);

    [[nodiscard]] Tensor MakeLiteral(int64_t i_integer_value);

    [[nodiscard]] Tensor MakeIdentifier(Tensor type, Name i_name);
    
    [[nodiscard]] bool AlwaysEqual(const Expression & i_first, const Expression & i_second);

    bool NameIs(const Tensor & i_tensor, const Name & i_name);

    bool NameIs(const Tensor & i_tensor, const ConstexprName & i_name);

    void ToSimplifiedStringForm(StringBuilder & i_dest, const Tensor & i_source);

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
