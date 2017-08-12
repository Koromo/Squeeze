#ifndef SQUEEZE_SQZCLASS_H
#define SQUEEZE_SQZCLASS_H

#include "sqztable.h"
#include "sqztableimpl.h"
#include "sqzstackop.h"
#include "sqzdef.h"
#include "sqzutil.h"
#include "squirrel/squirrel.h"
#include <type_traits>

namespace squeeze
{
    /** The Class object handle */
    template <class Class>
    class HClass : public HTableImpl
    {
    public:
        /** Construct */
        HClass() = default;

        /** Create a class object */
        explicit HClass(HVM vm)
        {
            vm_ = vm;
            const auto top = sq_gettop(vm_);
            sq_newclass(vm_, SQFalse);
            sq_getstackobj(vm_, -1, &obj_);
            sq_addref(vm_, &obj_);
            sq_settop(vm_, top);
        }

        /** Create a class object with extend 'base' class */
        template <class U>
        explicit HClass(HClass<U> base)
        {
            vm_ = base.vm();
            const auto top = sq_gettop(vm_);
            pushValue(vm_, base);
            sq_newclass(vm_, SQTrue);
            sq_getstackobj(vm_, -1, &obj_);
            sq_addref(vm_, &obj_);
            sq_settop(vm_, top);
        }

        /** Add constructor */
        template <class... Args>
        HClass& ctor()
        {
            newClosure(SQZ_T("constructor"), CtorClosure<Args...>::ctor, false);
            return *this;
        }

        /** Add a member as a variable */
        template <class T>
        HClass& var(const string_t& name, T val, bool isStatic = false)
        {
            newSlot(name, val, isStatic);
            return *this;
        }

        /** Add a member as a table */
        HClass& table(const string_t& name, HTable table, bool isStatic = false)
        {
            newSlot(name, table, isStatic);
            return *this;
        }

        /** Add a member as a non-static function */
        template <class Return, class... Args>
        HClass& fun(const string_t& name, Return(Class::*fun)(Args...))
        {
            newClosure(name, Closure::fun<decltype(fun)>, false, fun);
            return *this;
        }

        /** Add a member as a static function */
        template <class Fun, class = std::enable_if_t<!std::is_member_function_pointer<Fun>::value>>
        HClass& fun(const string_t& name, Fun fun)
        {
            newClosure(name, Closure::fun<Fun>, true, fun);
            return *this;
        }

    private:
        template <class... Args>
        struct CtorClosure
        {
            static SQInteger ctor(HSQUIRRELVM vm)
            {
                const auto inst = instantiate(vm, MakeIndexSequence<sizeof...(Args)>());
                sq_setinstanceup(vm, 1, inst);
                sq_setreleasehook(vm, 1, releaseHook);
                return 0;
            }

            template <size_t... ArgIndices>
            static Class* instantiate(HSQUIRRELVM vm, IndexSequence<ArgIndices...>)
            {
                using A = std::tuple<Args...>;
                return new Class(
                        (
                            static_cast
                            <
                                std::tuple_element_t<ArgIndices, A>
                            >
                            (
                                getValue
                                <
                                    SqType<std::tuple_element_t<ArgIndices, A>>
                                >
                                (vm, ArgIndices + 2)
                            )
                        )...
                    );
            }

            static SQInteger releaseHook(SQUserPointer p, SQInteger)
            {
                delete static_cast<Class*>(p);
                return 0;
            }
        };

        struct Closure
        {
            template <class Fun>
            static SQInteger fun(HSQUIRRELVM vm)
            {
                Fun* fun;
                sq_getuserdata(vm, -1, reinterpret_cast<SQUserPointer*>(&fun), nullptr);

                Class* inst;
                sq_getinstanceup(vm, 1, reinterpret_cast<SQUserPointer*>(&inst), nullptr);

                return fetchAndCall(vm, inst, *fun, MakeIndexSequence<FunctionTraits<Fun>::arity>());
            }

            template <class Fun, size_t... ArgIndices>
            static auto fetchAndCall(HSQUIRRELVM vm, Class* inst, Fun fun, IndexSequence<ArgIndices...>)
                -> std::enable_if_t<std::is_void<ReturnType<Fun>>::value, SQInteger>
            {
                (inst->*fun)(
                    (
                        static_cast
                        <
                            ArgumentType<Fun, ArgIndices>
                        >
                        (
                            getValue
                            <
                                SqType<ArgumentType<Fun, ArgIndices>>
                            >(vm, ArgIndices + 2)
                        )
                    )...);
                return 0;
            }

            template <class Fun, size_t... ArgIndices>
            static auto fetchAndCall(HSQUIRRELVM vm, Class* inst, Fun fun, IndexSequence<ArgIndices...>)
                -> std::enable_if_t<!std::is_void<ReturnType<Fun>>::value, SQInteger>
            {
                ReturnType<Fun> ret =
                    (inst->*fun)(
                        (
                            static_cast
                            <
                                ArgumentType<Fun, ArgIndices>
                            >
                            (
                                getValue
                                <
                                    SqType<ArgumentType<Fun, ArgIndices>>
                                >(vm, ArgIndices + 2)
                            )
                        )...);

                pushValue(vm, sq(ret));
                return 1;
            }
        };
    };

    template <class T> struct ToSquirrel<HClass<T>> { using Type = HSQOBJECT; };
}

#endif
