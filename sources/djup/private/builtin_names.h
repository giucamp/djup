
//   Copyright Giuseppe Campana (giu.campana@gmail.com) 2021-2025.
// Distributed under the Boost Software License, Version 1.0.
//        (See accompanying file LICENSE or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <private/common.h>
#include <private/name.h>

namespace djup
{
    namespace builtin_names
    {
        constexpr ConstexprName TensorType("TensorType");
        constexpr ConstexprName Identifier("Identifier");
        constexpr ConstexprName Literal("Literal");
        constexpr ConstexprName Stack("Stack");
        constexpr ConstexprName Tuple("Tuple");
        constexpr ConstexprName Namespace("Namespace");
        constexpr ConstexprName SubstitutionAxiom("SubstitutionAxiom");
        constexpr ConstexprName Is("Is");
        
        constexpr ConstexprName Any("any");
        constexpr ConstexprName Bool("bool");
        constexpr ConstexprName Int("int");

        constexpr ConstexprName RepetitionsZeroToMany("RepetitionsZeroToMany");
        constexpr ConstexprName RepetitionsOneToMany("RepetitionsOneToMany");
        constexpr ConstexprName RepetitionsZeroToOne("RepetitionsZeroToOne");

        constexpr ConstexprName AssociativeIdentifier("AssociativeIdentifier");
        
        constexpr ConstexprName If("If");
        constexpr ConstexprName Add("Add");
        constexpr ConstexprName Mul("Mul");
        constexpr ConstexprName Pow("Pow");
        constexpr ConstexprName And("And");
        constexpr ConstexprName Or("Or");
        constexpr ConstexprName Not("Not");
        constexpr ConstexprName Equal("Equal");
        constexpr ConstexprName Less("Less");
    }
}
