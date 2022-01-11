
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <core/span.h>
#include <core/hash.h>
#include <core/diagnostic.h>

namespace djup
{
    enum class ScalarType
    {
        Any                                 = 0b11111111,
            Number                          = 0b00111111,
                Bool                        = 0b00000001,
                Complex                     = 0b00111110,
                    Real                    = 0b00111100,
                        Integer             = 0b00110000,
    };

    template <typename TYPE>
        constexpr ScalarType GetScalarType()
    {
        if constexpr (std::is_same_v<TYPE, double>)
            return ScalarType::Rational;
        else if constexpr (std::is_same_v<TYPE, int64_t>)
            return ScalarType::Integer;
        else if constexpr (std::is_same_v<TYPE, bool>)
            return ScalarType::Bool;
    }

    ScalarType GetSmallestCommonSuperset(Span<const ScalarType> i_elements);

    /** Returns whether the first set type may be equivalent or a subset of
        the second type. If any of the set is NumericSet::Number, the result
        is always true.

        assert(MayBeSupersetOf(NumericSet::Real, NumericSet::Real));
        assert(MayBeSupersetOf(NumericSet::Real, NumericSet::Complex));
    */
    constexpr bool MayBeSupersetOf(ScalarType i_what, ScalarType i_candidate_superset) noexcept
    {
        return (static_cast<int>(i_what) & static_cast<int>(i_candidate_superset)) != 0;
    }

    constexpr bool IsSupersetOf(ScalarType i_what, ScalarType i_candidate_superset) noexcept
    {
        return (static_cast<int>(i_what) & static_cast<int>(i_candidate_superset)) == static_cast<int>(i_what);
    }

    template <> struct CharWriter<ScalarType>
    {
        constexpr void operator() (CharBufferView & i_dest, ScalarType i_source) noexcept
        {
            switch (i_source)
            {
                case ScalarType::Number:   i_dest << "number";  break;
                case ScalarType::Bool:     i_dest << "bool";    break;
                case ScalarType::Integer:  i_dest << "int";     break;
                case ScalarType::Real:     i_dest << "real";    break;
                case ScalarType::Complex:  i_dest << "complex"; break;
                default: Error("Unrecognized scalar type ", static_cast<int>(i_source));
            }
        }
    };

} // namespace djub
