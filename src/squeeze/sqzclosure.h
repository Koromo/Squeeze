#ifndef SQUEEZE_SQZCLOSURE_H
#define SQUEEZE_SQZCLOSURE_H

#include "sqzstackop.h"
#include "sqzutil.h"
#include "squirrel/squirrel.h"
#include <type_traits>
#include <tuple>

namespace squeeze
{
    /** Fetch and call a function */
    template <class Return, class Arguments>
    struct Fetch
    {
        using R = Return;
        using A = Arguments;
        enum { arity = std::tuple_size<A>::value };

        template <class Fun>
        static R call(HSQUIRRELVM vm, Fun fun)
        {
            return freefun(vm, fun, MakeIndexSequence<arity>());
        }

        template <class Fun, class Class>
        static R call(HSQUIRRELVM vm, Fun fun, Class* inst)
        {
            return memfun(vm, fun, inst, MakeIndexSequence<arity>());
        }

        template <class Fun, size_t... I>
        static R freefun(HSQUIRRELVM vm, Fun fun, IndexSequence<I...>)
        {
            return fun(getValue<std::tuple_element_t<I, A>>(vm, I + 2)...);
        }

        template <class Fun, class Class, size_t... I>
        static R memfun(HSQUIRRELVM vm, Fun fun, Class* inst, IndexSequence<I...>)
        {
            return (inst->*fun)(getValue<std::tuple_element_t<I, A>>(vm, I + 2)...); 
        }
    };

    /** A closure for constructors */
    template <class Class, class Arguments>
    struct CtorClosure
    {
        using A = Arguments;
        enum { arity = std::tuple_size<A>::value };

        static SQInteger ctor(HSQUIRRELVM vm)
        {
            const auto inst = instantiate(vm, MakeIndexSequence<arity>());
            sq_setinstanceup(vm, 1, inst);
            sq_setreleasehook(vm, 1, releaseHook);
            return 0;
        }

        template <size_t... I>
        static Class* instantiate(HSQUIRRELVM vm, IndexSequence<I...>)
        {
            return new Class(getValue<std::tuple_element_t<I, A>>(vm, I + 2)...);
        }

        static SQInteger releaseHook(SQUserPointer p, SQInteger)
        {
            delete static_cast<Class*>(p);
            return 0;
        }
    };

    /** A closure for free function embeddings. */
    template <class Fun>
    struct Closure
    {
        //using R = ReturnType<Fun>;
        using A = typename FunctionTraits<Fun>::Arguments;
        enum { arity = std::tuple_size<A>::value };

        static SQInteger fun(HSQUIRRELVM vm)
        {
            Fun* fun;
            sq_getuserdata(vm, -1, reinterpret_cast<SQUserPointer*>(&fun), nullptr);
            return fetch<ReturnType<Fun>>(vm, *fun, MakeIndexSequence<arity>());
        }

        template <class R, class Fun, size_t... I>
        static auto fetch(HSQUIRRELVM vm, Fun fun, IndexSequence<I...>)
            -> std::enable_if_t<std::is_void<R>::value, SQInteger>
        {
            Fetch<void, A>::call(vm, fun);
            return 0;
        }

        template <class R, class Fun, size_t... I>
        static auto fetch(HSQUIRRELVM vm, Fun fun, IndexSequence<I...>)
            -> std::enable_if_t<!std::is_void<R>::value && std::is_class<R>::value, SQInteger>
        {
            SQChar* classKey;
            sq_getuserdata(vm, -2, reinterpret_cast<SQUserPointer*>(&classKey), nullptr);

            HSQOBJECT env;
            sq_getstackobj(vm, 1, &env);
            sq_addref(vm, &env);

            if (!createClassInstance(vm, env, classKey, Fetch<R, A>::call(vm, fun)))
            {
                sq_release(vm, &env);
                failed<CallFailed>(vm, "fetch() failed.");
            }

            sq_release(vm, &env);
            return 1;
        }

        template <class R, class Fun, size_t... I>
        static auto fetch(HSQUIRRELVM vm, Fun fun, IndexSequence<I...>)
            -> std::enable_if_t<!std::is_void<R>::value && !std::is_class<R>::value, SQInteger>
        {
            const auto r = Fetch<R, A>::call(vm, fun);
            pushValue(vm, r);
            return 1;
        }
    };

    /** A closure for member function embeddings. */
    template <class Class, class Fun>
    struct MemClosure
    {
        //using R = ReturnType<Fun>;
        using A = typename FunctionTraits<Fun>::Arguments;
        enum { arity = std::tuple_size<A>::value };

        static SQInteger fun(HSQUIRRELVM vm)
        {
            Fun* fun;
            sq_getuserdata(vm, -1, reinterpret_cast<SQUserPointer*>(&fun), nullptr);

            Class* inst;
            sq_getinstanceup(vm, 1, reinterpret_cast<SQUserPointer*>(&inst), nullptr);

            return fetch<ReturnType<Fun>>(vm, *fun, inst, MakeIndexSequence<arity>());
        }

        template <class R, class Fun, size_t... I>
        static auto fetch(HSQUIRRELVM vm, Fun fun, Class* inst, IndexSequence<I...>)
            -> std::enable_if_t<std::is_void<R>::value, SQInteger>
        {
            Fetch<void, A>::call(vm, fun, inst);
            return 0;
        }

        template <class R, class Fun, size_t... I>
        static auto fetch(HSQUIRRELVM vm, Fun fun, Class* inst, IndexSequence<I...>)
            -> std::enable_if_t<!std::is_void<R>::value && std::is_class<R>::value, SQInteger>
        {
            const SQChar* classKey;
            sq_getstring(vm, -2, &classKey);

            HSQOBJECT env;
            sq_getstackobj(vm, 1, &env);
            sq_addref(vm, &env);

            if (!createClassInstance(vm, env, classKey, Fetch<R, A>::call(vm, fun, inst)))
            {
                sq_release(vm, &env);
                failed<CallFailed>(vm, "fetch() failed.");
            }

            sq_release(vm, &env);
            return 1;
        }

        template <class R, class Fun, size_t... I>
        static auto fetch(HSQUIRRELVM vm, Fun fun, Class* inst, IndexSequence<I...>)
            -> std::enable_if_t<!std::is_void<R>::value && !std::is_class<R>::value, SQInteger>
        {
            const auto r = Fetch<R, A>::call(vm, fun, inst);
            pushValue(vm, r);
            return 1;
        }
    };
}

#endif
