#ifndef SQUEEZE_SQZUTIL_H
#define SQUEEZE_SQZUTIL_H

#include "sqzdef.h"
#include "sqzvm.h"
#include <squirrel.h>
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
            enum { arity = sizeof...(Args) };
            using ReturnType = R;
            using Arguments = std::tuple<Args...>;
        };

        template <class R, class... Args>
        struct FunctionTraitsImpl<R(*)(Args...)> : FunctionTraitsImpl<R(Args...)> {};

        template <class C, class R, class... Args>
        struct FunctionTraitsImpl<R(C::*)(Args...)> : FunctionTraitsImpl<R(Args...)>
        {
            using ClassType = C;
        };

        template <class C, class R, class... Args>
        struct FunctionTraitsImpl<R(C::*)(Args...) const> : FunctionTraitsImpl<R(C::*)(Args...)> {};
    }

    /** The function traits. */
    template <class F>
    using FunctionTraits = typename detail::FunctionTraitsImpl<std::decay_t<F>>;

    /** A shortcut to the function traits. */
    template <class F>
    using ReturnType = typename FunctionTraits<F>::ReturnType;

    /// ditto
    template <class F>
    using Arguments = typename FunctionTraits<F>::Arguments;

    /// ditto
    template <class F, size_t N>
    using ArgumentType = std::tuple_element_t<N, Arguments<F>>;

    /// ditto
    template <class F>
    using ClassType = typename FunctionTraits<F>::ClassType;

    /** The IndexSequence class. */
    template <size_t... Indices>
    struct IndexSequence
    {
        static constexpr size_t length = sizeof...(Indices);
    };

    namespace detail
    {
        template <size_t N, size_t Extend, size_t Step, size_t... Indices>
        struct MakeIndicesImpl : MakeIndicesImpl<N - 1, Extend + Step, Step, Indices..., Extend>
        {
        };

        template <size_t Extend, size_t Step, size_t... Indices>
        struct MakeIndicesImpl<0, Extend, Step, Indices...>
        {
            using Type = IndexSequence<Indices...>;
        };
    }

    /** Create the IndexSequence<0, ... , N-1>. */
    template <size_t N>
    using MakeIndices = typename detail::MakeIndicesImpl<N, 0, 1>::Type;

    /** Call a callable object with arguments. */
    template <class R, class C, class T, class... Args>
    auto call(R C::* f, T&& t, Args&&... args)
        -> std::enable_if_t
        <
            std::is_member_function_pointer<decltype(f)>::value,
            decltype((std::forward<T>(t).*f)(std::forward<Args>(args)...))
        >
    {
        return (std::forward<T>(t).*f)(std::forward<Args>(args)...);
    }

    /// ditto
    template <class R, class C, class T, class... Args>
    auto call(R C::* f, T* t, Args&&... args)
        -> std::enable_if_t
        <
            std::is_member_function_pointer<decltype(f)>::value,
            decltype((t->*f)(std::forward<Args>(args)...))
        >
    {
        return (t->*f)(std::forward<Args>(args)...);
    }

    /// ditto
    template <class R, class C, class T>
    auto call(R C::* f, T&& t)
        -> std::enable_if_t
        <
            std::is_member_object_pointer<decltype(f)>::value,
            decltype(std::forward<T>(t).*f)
        >
    {
        return std::forward<T>(t).*f;
    }

    /// ditto
    template <class R, class C, class T>
    auto call(R C::* f, T* t)
        -> std::enable_if_t
        <
            std::is_member_object_pointer<decltype(f)>::value,
            decltype(t->*f)
        >
    {
        return t->*f;
    }

    /// ditto
    template <class F, class... Args>
    auto call(F&& f, Args&&... args)
        -> decltype(std::forward<F>(f)(std::forward<Args>(args)...))
    {
        return std::forward<F>(f)(std::forward<Args>(args)...);
    }

    /** Convert to unicode character set. */
    inline std::string narrow(const std::string& s)
    {
        return s;
    }

    /// ditto
    inline std::string narrow(const std::wstring& s)
    {
        const auto length = s.length();
        std::vector<char> multi(length + 1, '\0');
        std::wcstombs(multi.data(), s.data(), length);
        return multi.data();
    }

    /** Obtain last error message if exists. */
    inline string_t lastError(HSQUIRRELVM vm, const string_t& defaultMessage = SQZ_T(""))
    {
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

    /** Throw an error. */
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

    namespace detail
    {
        template <class Conv, class F, class... Args>
        struct WrappedCall
        {
            F f;
            string_t classKey;
            Conv operator()(Args... args) const
            {
                return{ call(f, args...), classKey };
            }
        };

        template <class F, size_t... I>
        auto wrapConvImpl(F&& f, const string_t& classKey, IndexSequence<I...>)
            -> WrappedCall<ClassConv<ReturnType<F>>, F, ArgumentType<F, I>...>
        {
            return WrappedCall<ClassConv<ReturnType<F>>, F, ArgumentType<F, I>...>{ std::forward<F>(f), classKey };
        }

        template <class R, class C, size_t... I>
        auto wrapConvImpl(R C::* f, const string_t& classKey, IndexSequence<I...>)
            -> WrappedCall<ClassConv<R>, decltype(f), ArgumentType<decltype(f), I>...>
        {
            return WrappedCall<ClassConv<R>, decltype(f), C*, ArgumentType<decltype(f), I>...>{ std::forward<F>(f), classKey };
        }
    }

    /** Wrap a function and return new function witch returns the ClassConv. */
    template <class F, class R = ReturnType<F>, class = std::enable_if_t<std::is_class<R>::value && !std::is_same<R, string_t>::value>>
    auto wrapConv(F&& f, const string_t& classKey)
        -> decltype (detail::wrapConvImpl(std::forward<F>(f), classKey, MakeIndices<FunctionTraits<F>::arity>()))
    {
        return detail::wrapConvImpl(std::forward<F>(f), classKey, MakeIndices<FunctionTraits<F>::arity>());
    }
}

#endif