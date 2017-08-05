#ifndef SQUEEZE_SQZTRAITS_H
#define SQUEEZE_SQZTRAITS_H

#include <tuple>

namespace squeeze
{
    template <class... Args>
    struct TypeList
    {
        static constexpr size_t length = sizeof...(Args);

        template <size_t N>
        using Type = typename std::tuple_element<N, std::tuple<Args...>>::type;
    };

    template <class T>
    struct SignatureOf;

    template <class R, class... Args>
    struct SignatureOf<R(*)(Args...)> // Function
    {
        using Result = R;
        using Arguments = TypeList<Args...>;
    };

    template <class C, class R, class... Args>
    struct SignatureOf<R(C::*)(Args...)> // Member function
    {
        using Result = R;
        using Arguments = TypeList<Args...>;
    };

    template <class C, class R, class... Args>
    struct SignatureOf<R(C::*)(Args...) const> // Const member function
    {
        using Result = R;
        using Arguments = TypeList<Args...>;
    };

    template <class C>
    struct SignatureOf : SignatureOf<decltype(&C::operator())> // Functor
    {
    };

    template <class Fun>
    using ResultType = typename SignatureOf<Fun>::Result;

    template <class Fun>
    using ArgumentTypes = typename SignatureOf<Fun>::Arguments;

    template <size_t... Indices>
    struct IndexSequence
    {
        static constexpr size_t length = sizeof...(Indices);
    };

    namespace detail
    {
        template <size_t N, size_t I, size_t... Indices>
        struct MakeIndexSeq : MakeIndexSeq<N - 1, I + 1, Indices..., I>
        {
        };

        template <size_t I, size_t... Indices>
        struct MakeIndexSeq<0, I, Indices...>
        {
            using Type = IndexSequence<Indices...>;
        };
    }

    template <size_t N>
    auto makeIndexSequence()
        -> decltype(detail::MakeIndexSeq<N, 0>::Type())
    {
        return detail::MakeIndexSeq<N, 0>::Type();
    }
}

#endif