
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <core/span.h>
#include <core/hash.h>
#include <core/diagnostic.h>

namespace djup
{
    enum class Domain
    { 
        Any                                 = 0b11111111,
            Tuple                           = 0b01000000,
            Number                          = 0b00111111, 
                Bool                        = 0b00000001,
                Complex                     = 0b00111110,
                    Real                    = 0b00111100,
                        Rational            = 0b00111000,
                            Integer         = 0b00110000,
                                Natural     = 0b00100000,
    };

    template <typename TYPE>
        constexpr Domain GetDomain()
    {
        if constexpr (std::is_same_v<TYPE, double>)
            return Domain::Rational;
        else if constexpr (std::is_same_v<TYPE, int64_t>)
            return Domain::Integer;
        else if constexpr (std::is_same_v<TYPE, bool>)
            return Domain::Bool;
    }

    Domain GetSmallestCommonSuperset(Span<const Domain> i_elements);

    /** Returns whether the first set type may be equivalent or a subset of 
        the second type. If any of the set is NumericSet::Number, the result
        is always true. 
        
        assert(MayBeSupersetOf(NumericSet::Real, NumericSet::Real));
        assert(MayBeSupersetOf(NumericSet::Real, NumericSet::Complex));    
    */
    constexpr bool MayBeSupersetOf(Domain i_what, Domain i_candidate_superset) noexcept
    {
        return (static_cast<int>(i_what) & static_cast<int>(i_candidate_superset)) != 0;
    }

    constexpr bool IsSupersetOf(Domain i_what, Domain i_candidate_superset) noexcept
    {
        return (static_cast<int>(i_what) & static_cast<int>(i_candidate_superset)) == static_cast<int>(i_what);
    }

    template <> struct CharWriter<Domain>
    {
        constexpr void operator() (CharBufferView & i_dest, Domain i_source) noexcept
        {
            switch (i_source)
            {
                case Domain::Number:   i_dest << "number";  break;
                case Domain::Bool:     i_dest << "bool";    break;
                case Domain::Natural:  i_dest << "natural"; break;
                case Domain::Integer:  i_dest << "int";     break;
                case Domain::Rational: i_dest << "rational";break;
                case Domain::Real:     i_dest << "real";    break;
                case Domain::Complex:  i_dest << "complex"; break;      
                default: Error("Unrecognized scalar type ", static_cast<int>(i_source));
            }
        }
    };

} // namespace djub
