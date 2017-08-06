#ifndef SQUEEZE_SQZTRAITS_H
#define SQUEEZE_SQZTRAITS_H

#include <tuple>
#include <type_traits>

namespace squeeze
{
    namespace detail
    {
        template <class T>
        struct FunctionTraitsImpl : FunctionTraitsImpl<decltype(&T::operator())> {};

        template <class R, class... Args>
        struct FunctionTraitsImpl<R(Args...)>
        {
            using ReturnType = R;

            enum { arity = sizeof...(Args) };

            using Arguments = std::tuple<Args...>;

            template <size_t N>
            using ArgumentType = std::enable_if_t<(N < arity), std::tuple_element_t<N, Arguments>>;
        };

        template <class R, class... Args>
        struct FunctionTraitsImpl<R(*)(Args...)> : FunctionTraitsImpl<R(Args...)> {};

        template <class C, class R, class... Args>
        struct FunctionTraitsImpl<R(C::*)(Args...)> : FunctionTraitsImpl<R(Args...)> {};

        template <class C, class R, class... Args>
        struct FunctionTraitsImpl<R(C::*)(Args...) const> : FunctionTraitsImpl<R(Args...)> {};
    }

    /// The FunctionTraits
    template <class Fun>
    using FunctionTraits = typename detail::FunctionTraitsImpl<std::decay_t<Fun>>;

    /// Shortcut to FunctionTraits
    template <class Fun>
    using ReturnType = typename FunctionTraits<Fun>::ReturnType;

    /// ditto
    template <class Fun, size_t N>
    using ArgumentType = std::enable_if_t<(N < FunctionTraits<Fun>::arity), std::tuple_element_t<N, typename FunctionTraits<Fun>::Arguments>>;

    template <size_t... Indices>
    struct IndexSequence
    {
        static constexpr size_t length = sizeof...(Indices);
    };

    namespace detail
    {
        template <size_t N, size_t Extend, size_t Step, size_t... Indices>
        struct MakeIndices : MakeIndices<N - 1, Extend + Step, Step, Indices..., Extend>
        {
        };

        template <size_t Extend, size_t Step, size_t... Indices>
        struct MakeIndices<0, Extend, Step, Indices...>
        {
            using Type = IndexSequence<Indices...>;
        };
    }

    template <size_t N>
    using MakeIndexSequence = typename detail::MakeIndices<N, 0, 1>::Type;
}

#endif