#ifndef SQUEEZE_SQZCLOSURE_H
#define SQUEEZE_SQZCLOSURE_H

#include "sqzstackop.h"
#include "sqzutil.h"
#include "squirrel/squirrel.h"
#include <type_traits>
#include <tuple>

namespace squeeze
{
    namespace detail
    {
        struct VoidType {};
    }

    /**
    Create a class instance object and push it to the stack.
    The class is converted to the object required the move or copy constructor (move has priority).
    */
    template <class Class>
    bool pushClassInstance(HSQUIRRELVM vm, HSQOBJECT env, const SQChar* classKey, Class&& inst)
    {
        const auto top = sq_gettop(vm);

        sq_pushobject(vm, env);
        sq_pushstring(vm, classKey, -1);
        if (SQ_FAILED(sq_rawget(vm, -2)))
        {
            sq_settop(vm, top);
            sq_pushroottable(vm);
            sq_pushstring(vm, classKey, -1);

            if (SQ_FAILED(sq_rawget(vm, -2)))
            {
                sq_settop(vm, top);
                return false;
            }
        }
        if (SQ_FAILED(sq_createinstance(vm, -1)))
        {
            sq_settop(vm, top);
            return false;
        }

        sq_remove(vm, -3); // Remove the table (roottable or env).
        sq_remove(vm, -2); // Remove the class object.

        const auto copy = new Class(std::move(inst));
        if (SQ_FAILED(sq_setinstanceup(vm, -1, copy)))
        {
            delete copy;
            sq_settop(vm, top);
            return false;
        }
        sq_setreleasehook(vm, -1, CtorClosure<Class>::releaseHook);

        return true;
    }

    /** Push the return value */
    template <class R>
    auto pushReturn(HSQUIRRELVM vm, R ret)
        -> std::enable_if_t<std::is_same<R, detail::VoidType>::value, SQInteger>
    {
        return 0;
    }

    /// ditto
    template <class R>
    auto pushReturn(HSQUIRRELVM vm, R ret)
        -> std::enable_if_t<!std::is_same<R, detail::VoidType>::value, SQInteger>
    {
        pushValue(vm, ret);
        return 1;
    }

    /// ditto
    template <class T>
    SQInteger pushReturn(HSQUIRRELVM vm, ClassConv<T>&& ret)
    {
        HSQOBJECT env;
        sq_getstackobj(vm, 1, &env);
        sq_addref(vm, &env);
        if (!pushClassInstance(vm, env, ret.classKey.c_str(), std::move(ret.v)))
        {
            sq_release(vm, &env);
            failed<CallFailed>(vm, "Failed to create instance.");
        }

        sq_release(vm, &env);
        return 1;
    }

    namespace detail
    {
        template <size_t offset, size_t... I, class F, class... Heads, class R = ReturnType<F>>
        auto fetchImpl(IndexSequence<I...>, HSQUIRRELVM vm, F&& f, Heads&&... heads)
            -> std::enable_if_t<!std::is_same<R, void>::value, R>
        {
            return call(std::forward<F>(f), std::forward<Heads>(heads)..., getValue<ArgumentType<F, offset + I>>(vm, I + 2)...);
        }

        template <size_t offset, size_t... I, class F, class... Heads, class R = ReturnType<F>>
        auto fetchImpl(IndexSequence<I...>, HSQUIRRELVM vm, F&& f, Heads&&... heads)
            -> std::enable_if_t<std::is_same<R, void>::value, VoidType>
        {
            call(std::forward<F>(f), std::forward<Heads>(heads)..., getValue<ArgumentType<F, offset + I>>(vm, I + 2)...);
            return{};
        }
    }

    /** Fetch arguments from the stack and call the function */
    template <class F, size_t arity = FunctionTraits<F>::arity>
    auto fetch(HSQUIRRELVM vm, F&& f)
        -> decltype(detail::fetchImpl<0>(MakeIndices<arity>(), vm, std::forward<F>(f)))
    {
        return detail::fetchImpl<0>(MakeIndices<arity>(), vm, std::forward<F>(f));
    }

    /// ditto
    template <class F, class Head, size_t arity = FunctionTraits<F>::arity, class = std::enable_if_t<(arity > 0)>>
    auto fetch(HSQUIRRELVM vm, F&& f, Head&& head)
        -> decltype(detail::fetchImpl<1>(MakeIndices<arity - 1>(), vm, std::forward<F>(f), std::forward<Head>(head)))
    {
        return detail::fetchImpl<1>(MakeIndices<arity - 1>(), vm, std::forward<F>(f), std::forward<Head>(head));
    }

    /// ditto
    template <class R, class C, class Head>
    auto fetch(HSQUIRRELVM vm, R C::* f, Head&& head)
        -> decltype(detail::fetchImpl<0>(MakeIndices<FunctionTraits<decltype(f)>::arity>(), vm, std::forward<decltype(f)>(f), std::forward<Head>(head)))
    {
        return detail::fetchImpl<0>(MakeIndices<FunctionTraits<decltype(f)>::arity>(), vm, std::forward<decltype(f)>(f), std::forward<Head>(head));
    }

    /** A closure for constructors */
    template <class Class>
    struct CtorClosure
    {
        template < class... Args>
        static SQInteger ctor(HSQUIRRELVM vm)
        {
            using ArgTuple = std::tuple<Args...>;
            const auto arity = sizeof...(Args);

            const auto inst = instantiate<ArgTuple>(vm, MakeIndices<arity>());
            sq_setinstanceup(vm, 1, inst);
            sq_setreleasehook(vm, 1, releaseHook);
            return 0;
        }

        template <class Args, size_t... I>
        static Class* instantiate(HSQUIRRELVM vm, IndexSequence<I...>)
        {
            return new Class(getValue<std::tuple_element_t<I, Args>>(vm, I + 2)...);
        }

        static SQInteger releaseHook(SQUserPointer p, SQInteger)
        {
            delete static_cast<Class*>(p);
            return 0;
        }
    };

    /** Closures for function embeddings. */
    struct Closure
    {
        template <class F>
        static SQInteger fun(HSQUIRRELVM vm)
        {
            F* f;
            sq_getuserdata(vm, -1, reinterpret_cast<SQUserPointer*>(&f), nullptr);

            auto&& ret = fetch(vm, *f);
            return pushReturn(vm, std::move(ret));
        }

        template <class F, class Class>
        static SQInteger memfun(HSQUIRRELVM vm)
        {
            F* f;
            sq_getuserdata(vm, -1, reinterpret_cast<SQUserPointer*>(&f), nullptr);

            Class* inst;
            sq_getinstanceup(vm, 1, reinterpret_cast<SQUserPointer*>(&inst), nullptr);

            auto&& ret = fetch(vm, *f, inst);
            return pushReturn(vm, std::move(ret));
        }

        static SQInteger opSet(HSQUIRRELVM vm)
        {
            const auto top = sq_gettop(vm);

            sq_push(vm, 2);
            if (SQ_FAILED(sq_get(vm, -2)))
            {
                sq_settop(vm, top);
                failed<CallFailed>(vm, "sq_get() failed.");
            }

            sq_push(vm, 1);
            sq_push(vm, 3);

            if (sq_call(vm, 2, SQFalse, SQTrue))
            {
                sq_settop(vm, top);
                failed<CallFailed>(vm, "sq_call() failed.");
            }
            sq_settop(vm, top);
            return 0;
        }

        static SQInteger opGet(HSQUIRRELVM vm)
        {
            const auto top = sq_gettop(vm);

            sq_push(vm, 2);
            if (SQ_FAILED(sq_get(vm, -2)))
            {
                sq_settop(vm, top);
                failed<CallFailed>(vm, "sq_get() failed.");
            }

            sq_push(vm, 1);

            if (sq_call(vm, 1, SQTrue, SQTrue))
            {
                sq_settop(vm, top);
                failed<CallFailed>(vm, "sq_call() failed.");
            }
            sq_remove(vm, -2);
            return 1;
        }
    };
}

#endif
