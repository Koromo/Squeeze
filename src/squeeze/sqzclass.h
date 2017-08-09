#ifndef SQUEEZE_SQZCLASS_H
#define SQUEEZE_SQZCLASS_H

#include "sqztable.h"
#include "sqztableimpl.h"
#include "sqzstackop.h"
#include "sqzdef.h"
#include "sqztraits.h"
#include "squirrel/squirrel.h"
#include <type_traits>

namespace squeeze
{
    /// The Class handle
    template <class Class>
    class HClass : public HTableImpl
    {
    public:
        /// Construct
        HClass() = default;

        /// Create a class
        explicit HClass(HVM vm)
        {
            vm_ = vm;

            const auto top = sq_gettop(vm_);
            sq_newclass(vm_, SQFalse);
            sq_getstackobj(vm_, -1, &obj_);
            sq_addref(vm_, &obj_);
            sq_settop(vm_, top);
        }

        /// ditto
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

        /// Define the constructor
        template <class... Args>
        HClass& ctor()
        {
            newClosure(SQZ_T("constructor"), CtorClosure<Args...>::ctor, false);
            return *this;
        }

        /// Define a variable
        template <class T>
        HClass& var(string_t name, T val, bool isStatic = false)
        {
            newSlot(name, val, sq(isStatic));
            return *this;
        }

        /// Define a table
        HClass& table(string_t name, HTable t, bool isStatic = false)
        {
            newSlot(name, t, sq(isStatic));
            return *this;
        }

        /// Define a function
        template <class Fun>
        auto fun(string_t name, Fun fun)
            -> std::enable_if_t<std::is_member_function_pointer<Fun>::value, HClass&>
        {
            newClosure(name, Closure::fun<Fun>, false, fun);
            return *this;
        }

        template <class Fun>
        auto fun(string_t name, Fun fun, bool isStatic = false)
            -> std::enable_if_t<!std::is_member_function_pointer<Fun>::value, HClass&>
        {
            newClosure(name, HTable::Closure::fun<Fun>, isStatic, fun);
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
                    cpp(
                        getValue<std::tuple_element_t<ArgIndices, A>>(vm, ArgIndices + 2)
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
                -> std::enable_if_t<std::is_same<ReturnType<Fun>, void>::value, SQInteger>
            {
                (inst->*fun)(
                    cpp(
                        getValue<SqType<ArgumentType<Fun, ArgIndices>>>(vm, ArgIndices + 2)
                        )...
                    );
                return 0;
            }

            template <class Fun, size_t... ArgIndices>
            static auto fetchAndCall(HSQUIRRELVM vm, Class* inst, Fun fun, IndexSequence<ArgIndices...>)
                -> std::enable_if_t<!std::is_same<ReturnType<Fun>, void>::value, SQInteger>
            {
                pushValue(
                    vm,
                    sq(
                        (inst->*fun)(
                            cpp(
                                getValue<SqType<ArgumentType<Fun, ArgIndices>>>(vm, ArgIndices + 2)
                                )...
                            )
                        )
                    );
                return 1;
            }
        };
    };
}

#endif
