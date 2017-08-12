#ifndef SQUEEZE_SQZUTIL_H
#define SQUEEZE_SQZUTIL_H

#include "sqzvm.h"
#include "squirrel/squirrel.h"
#include <tuple>
#include <type_traits>
#include <vector>
#include <cctype>
#include <cwctype>
#include <algorithm>

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

    inline string_t lastError(HSQUIRRELVM vm, const string_t& defaultMessage = SQZ_T("")) {
        sq_getlasterror(vm);
        if (sq_gettype(vm, -1) == OT_NULL)
        {
            sq_pop(vm, 1);
            return defaultMessage;
        }
        sq_tostring(vm, -1);
        const SQChar* err;
        sq_getstring(vm, -1, &err);
        sq_pop(vm, 2);
        return err;
    }

    inline std::string narrow(const std::string& s)
    {
        return s;
    }

    inline std::string narrow(const std::wstring& s)
    {
        const auto length = s.length();
        std::vector<char> multi(length + 1, '\0');
        std::wcstombs(multi.data(), s.data(), length);
        return multi.data();
    }

    template <class E>
    void failed(HSQUIRRELVM vm, const std::string& msg)
    {
        const auto s = lastError(vm);
        if (s.empty())
        {
            throw E(msg);
        }
        throw E(msg + " " + narrow(s));
    }
}

#endif