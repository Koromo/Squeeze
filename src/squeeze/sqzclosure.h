#ifndef SQUEEZE_SQZCLOSURE_H
#define SQUEEZE_SQZCLOSURE_H

#include "sqzstackop.h"
#include "sqzutil.h"
#include "squirrel/squirrel.h"
#include <type_traits>
#include <tuple>

namespace squeeze
{
    /** The void type */
    struct Void {};

    /**
    Create a class instance object and push it to the stack.
    The class in host code required the move or copy constructor (move has priority).
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
        sq_setreleasehook(vm, -1, CtorClosure<Class, std::tuple<>>::releaseHook);

        return true;
    }

    /** Push the return value */
    template <class R>
    auto pushReturn(HSQUIRRELVM vm, R ret)
        -> std::enable_if_t<std::is_same<R, Void>::value, SQInteger>
    {
        return 0;
    }

    template <class R>
    auto pushReturn(HSQUIRRELVM vm, R&& ret)
        -> std::enable_if_t<std::is_class<R>::value && !std::is_same<R, Void>::value, SQInteger>
    {
        SQChar* classKey;
        sq_getuserdata(vm, -2, reinterpret_cast<SQUserPointer*>(&classKey), nullptr);

        HSQOBJECT env;
        sq_getstackobj(vm, 1, &env);
        sq_addref(vm, &env);

        if (!pushClassInstance(vm, env, classKey, std::move(ret)))
        {
            sq_release(vm, &env);
            failed<CallFailed>(vm, "Failed to create instance.");
        }

        sq_release(vm, &env);
        return 1;
    }

    template <class R>
    auto pushReturn(HSQUIRRELVM vm, R ret)
        -> std::enable_if_t<!std::is_class<R>::value && !std::is_same<R, Void>::value, SQInteger>
    {
        pushValue(vm, ret);
        return 1;
    }

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

    template <class Arguments>
    struct Fetch<void, Arguments>
    {
        using A = Arguments;
        enum { arity = std::tuple_size<A>::value };

        template <class Fun>
        static Void call(HSQUIRRELVM vm, Fun fun)
        {
            freefun(vm, fun, MakeIndexSequence<arity>());
            return{};
        }

        template <class Fun, class Class>
        static Void call(HSQUIRRELVM vm, Fun fun, Class* inst)
        {
            memfun(vm, fun, inst, MakeIndexSequence<arity>());
            return{};
        }

        template <class Fun, size_t... I>
        static void freefun(HSQUIRRELVM vm, Fun fun, IndexSequence<I...>)
        {
            fun(getValue<std::tuple_element_t<I, A>>(vm, I + 2)...);
        }

        template <class Fun, class Class, size_t... I>
        static void memfun(HSQUIRRELVM vm, Fun fun, Class* inst, IndexSequence<I...>)
        {
            (inst->*fun)(getValue<std::tuple_element_t<I, A>>(vm, I + 2)...);
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
        using R = ReturnType<Fun>;
        using A = typename FunctionTraits<Fun>::Arguments;
        enum { arity = std::tuple_size<A>::value };

        static SQInteger fun(HSQUIRRELVM vm)
        {
            Fun* fun;
            sq_getuserdata(vm, -1, reinterpret_cast<SQUserPointer*>(&fun), nullptr);

            auto ret = Fetch<R, A>::call(vm, *fun);
            return pushReturn(vm, std::move(ret));
        }
    };

    /** A closure for member function embeddings. */
    template <class Class, class Fun = void(Class::*)()>
    struct MemClosure
    {
        using R = ReturnType<Fun>;
        using A = typename FunctionTraits<Fun>::Arguments;
        enum { arity = std::tuple_size<A>::value };

        static SQInteger fun(HSQUIRRELVM vm)
        {
            Fun* fun;
            sq_getuserdata(vm, -1, reinterpret_cast<SQUserPointer*>(&fun), nullptr);

            Class* inst;
            sq_getinstanceup(vm, 1, reinterpret_cast<SQUserPointer*>(&inst), nullptr);

            auto ret = Fetch<R, A>::call(vm, *fun, inst);
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
