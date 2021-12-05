
#pragma once
#include <type_traits>
#include <utility>
#include <string_view> // for std::data and std::size

namespace djup
{
    #ifdef __clang__
        template<typename... TYPES> struct MakeVoid_ { typedef void type;};
        template<typename... TYPES> using VoidT = typename MakeVoid_<TYPES...>::type;
    #else
        template<typename... TYPES> using VoidT = std::void_t<TYPES...>;
    #endif

    // ContainerElementTypeT
    template <typename CONTAINER, typename = VoidT<>>
        struct ContainerElementType { };
    template <typename CONTAINER>
        struct ContainerElementType<CONTAINER, VoidT<
        decltype(*std::begin(std::declval<CONTAINER&>())),
        decltype(std::begin(std::declval<CONTAINER&>()) != std::end(std::declval<CONTAINER&>())) > >
    { 
        using type = std::decay_t<decltype(*std::begin(std::declval<CONTAINER&>()))>;
    };
    template <typename TYPE> using ContainerElementTypeT = typename ContainerElementType<TYPE>::type;

    // trait IsComparable - detect whether has operators == and <
    template <typename TYPE, typename = VoidT<>> struct IsComparable : std::false_type { };
    template <typename TYPE> struct IsComparable< TYPE, VoidT<decltype(
        std::declval<TYPE const &>() < std::declval<TYPE const &>() ||
        std::declval<TYPE const &>() == std::declval<TYPE const &>()
    ) > > : std::true_type { };
    template <typename TYPE> using IsComparableT = typename IsComparable<TYPE>::type;
    template <typename TYPE> constexpr bool IsComparableV = IsComparable<TYPE>::value;

    // IsContainerV<CONTAINER, ELEMENT_TYPE>
    template <typename CONTAINER, typename = VoidT<>>
        struct IsContainer : std::false_type { };
    template <typename CONTAINER>
        struct IsContainer < CONTAINER, VoidT<
            decltype(std::begin(std::declval<CONTAINER&>()) != std::end(std::declval<CONTAINER&>()))
    > > : std::true_type{ };
    template <typename CONTAINER>
        constexpr bool IsContainerV = IsContainer<CONTAINER>::value;

    // IsContigousContainerV<CONTAINER, ELEMENT_TYPE>
    template <typename CONTAINER, typename = VoidT<>>
        struct IsContigousContainer : std::false_type { };
    template <typename CONTAINER>
        struct IsContigousContainer< CONTAINER, VoidT< std::enable_if_t<std::conjunction_v<

            // decltype(std::data(i_source_container)) must be convertible to ELEMENT_TYPE *
            std::is_convertible<decltype(std::data(std::declval<CONTAINER&>())), ContainerElementTypeT<CONTAINER>*>,

            // decltype(std::size(i_source_container)) must be convertible to size_t
            std::is_convertible<decltype(std::size(std::declval<CONTAINER&>())), size_t>,

            // decltype(std::data())(*)[] must be convertible to ELEMENT_TYPE (*)[]
            std::is_convertible<std::remove_pointer_t<decltype(std::data(std::declval<CONTAINER&>()))>(*)[], ContainerElementTypeT<CONTAINER>(*)[]>

    > > > > : std::true_type { };
    template <typename CONTAINER>
        constexpr bool IsContigousContainerV = IsContigousContainer<CONTAINER>::value;

    // IsContainerOfV<CONTAINER, ELEMENT_TYPE>
    template <typename CONTAINER, typename ELEMENT_TYPE, typename = VoidT<>>
        struct IsContainerOf : std::false_type { };
    template <typename CONTAINER, typename ELEMENT_TYPE>
        struct IsContainerOf < CONTAINER, ELEMENT_TYPE, VoidT<

            decltype(std::begin(std::declval<CONTAINER&>()) != std::end(std::declval<CONTAINER&>())),

            std::enable_if_t<std::is_same_v<
                std::decay_t<ELEMENT_TYPE>,
                std::decay_t<decltype(*std::begin(std::declval<CONTAINER&>()))>
            >>

    > > : std::true_type{ };
    template <typename CONTAINER, typename ELEMENT_TYPE>
        constexpr bool IsContainerOfV = IsContainerOf<CONTAINER, ELEMENT_TYPE>::value;

    // IsContigousContainerOfV<CONTAINER, ELEMENT_TYPE>
    template <typename CONTAINER, typename ELEMENT_TYPE, typename = VoidT<>>
        struct IsContigousContainerOf : std::false_type { };
    template <typename CONTAINER, typename ELEMENT_TYPE>
        struct IsContigousContainerOf< CONTAINER, ELEMENT_TYPE, VoidT< std::enable_if_t<std::conjunction_v<

            // decltype(std::data(i_source_container)) must be convertible to ELEMENT_TYPE *
            std::is_convertible<decltype(std::data(std::declval<CONTAINER&>())), ELEMENT_TYPE*>,

            // decltype(std::size(i_source_container)) must be convertible to size_t
            std::is_convertible<decltype(std::size(std::declval<CONTAINER&>())), size_t>,

            // decltype(std::data())(*)[] must be convertible to ELEMENT_TYPE (*)[]
            std::is_convertible<std::remove_pointer_t<decltype(std::data(std::declval<CONTAINER&>()))>(*)[], ELEMENT_TYPE(*)[]>

    > > > > : std::true_type { };
    template <typename CONTAINER, typename ELEMENT_TYPE>
        constexpr bool IsContigousContainerOfV = IsContigousContainerOf<CONTAINER, ELEMENT_TYPE>::value;

    // trait HasSize - detect whether std::size is defined
    template <typename TYPE, typename = VoidT<>> struct HasSize : std::false_type { };
    template <typename TYPE> struct HasSize< TYPE, VoidT<decltype(
        std::size(std::declval<TYPE const &>())
    ) > > : std::true_type { };
    template <typename TYPE> using HasSizeT = typename HasSize<TYPE>::type;
    template <typename TYPE> constexpr bool HasSizeV = HasSize<TYPE>::value;

    template <typename FIRST, typename...>
        using FirstOf = FIRST;

    template <typename TYPE>
        auto RemoveConst(TYPE * i_ptr) noexcept
    {
        return const_cast<std::remove_const_t<TYPE>*>(i_ptr);
    }
}
