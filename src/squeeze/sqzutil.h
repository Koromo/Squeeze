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
    using ArgumentType = std::enable_if_t<(N < FunctionTraits<F>::arity), std::tuple_element_t<N, Arguments<F>>>;

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

    namespace detail
    {
        template <class T>
        struct IsPointer : std::is_pointer<std::remove_reference_t<T>> {};
    }

    /** Call a callable object with arguments. */
    template <class R, class C, class T, class... Args>
    auto call(R C::* f, T&& t, Args&&... args)
        -> std::enable_if_t
        <
            std::is_member_function_pointer<decltype(f)>::value && !detail::IsPointer<T>::value,
            decltype((std::forward<T>(t).*f)(std::forward<Args>(args)...))
        >
    {
        return (std::forward<T>(t).*f)(std::forward<Args>(args)...);
    }

    template <class R, class C, class T, class... Args>
    auto call(R C::* f, T&& t, Args&&... args)
        -> std::enable_if_t
        <
            std::is_member_function_pointer<decltype(f)>::value && detail::IsPointer<T>::value,
            decltype((std::forward<T>(t)->*f)(std::forward<Args>(args)...))
        >
    {
        return (std::forward<T>(t)->*f)(std::forward<Args>(args)...);
    }

    template <class R, class C, class T, class... Args>
    auto call(R C::* f, T&& t, Args&&... args)
        -> std::enable_if_t
        <
            std::is_member_object_pointer<decltype(f)>::value && !detail::IsPointer<T>::value,
            decltype(std::forward<T>(t).*f)
        >
    {
        return std::forward<T>(t).*f;
    }

    template <class R, class C, class T, class... Args>
    auto call(R C::* f, T&& t, Args&&... args)
        -> std::enable_if_t
        <
            std::is_member_object_pointer<decltype(f)>::value && detail::IsPointer<T>::value,
            decltype(std::forward<T>(t)->*f)
        >
    {
        return std::forward<T>(t)->*f;
    }

    template <class F, class... Args>
    auto call(F&& f, Args&&... args)
        -> decltype(std::forward<F>(f)(std::forward<Args>(args)...))
    {
        return std::forward<F>(f)(std::forward<Args>(args)...);
    }

    template <class T>
    struct IsUserClass : std::conditional_t<std::is_class<T>::value && !std::is_same<T, string_t>::value, std::true_type, std::false_type> {};

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