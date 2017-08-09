#ifndef SQUEEZE_SQZTABLE_H
#define SQUEEZE_SQZTABLE_H

#include "sqztableimpl.h"
#include "sqzstackop.h"
#include "sqzdef.h"
#include "sqztraits.h"
#include "squirrel/squirrel.h"
#include <type_traits>

namespace squeeze
{
    template <class Class> class HClass;

    /// The Table handle
    class HTable : public HTableImpl
    {
    public:
        /// Construct
        HTable() = default;

        /// Create a table
        explicit HTable(HVM vm)
        {
            vm_ = vm;

            const auto top = sq_gettop(vm_);
            sq_newtable(vm_);
            sq_getstackobj(vm_, -1, &obj_);
            sq_addref(vm_, &obj_);
            sq_settop(vm_, top);
        }

        HTable(HVM vm, HSQOBJECT obj)
        {
            vm_ = vm;
            obj_ = obj;
            sq_addref(vm_, &obj_);
        }

        /// Create a clone
        HTable clone()
        {
            HTable clo(vm_);

            const auto top = sq_gettop(vm_);
            pushValue(vm_, obj_, clo.obj_, nullptr);
            while (SQ_SUCCEEDED(sq_next(vm_, -3)))
            {
                sq_newslot(vm_, -4, SQFalse);
            }
            sq_settop(vm_, top);

            return clo;
        }

        /// Set a variable
        template <class T>
        HTable& var(string_t name, T val)
        {
            newSlot(name, val, false);
            return *this;
        }

        /// Set a table
        HTable& table(string_t name, HTable t)
        {
            newSlot(name, t, false);
            return *this;
        }

        /// Set a class
        template <class Class>
        HTable& klass(string_t name, HClass<Class> c);

        /// Set a function
        template <class Fun>
        HTable& fun(string_t name, Fun fun)
        {
            newClosure(name, Closure::fun<Fun>, false, fun);
            return *this;
        }

        /// Call a function
        template <class Result, class... Args>
        Result call(string_t name, HTable env, Args&&... args)
        {
            return HTableImpl::call<Result>(name, env, std::forward<Args>(args)...);
        }

        struct Closure
        {
            template <class Fun>
            static SQInteger fun(HSQUIRRELVM vm)
            {
                Fun* fun;
                sq_getuserdata(vm, -1, reinterpret_cast<SQUserPointer*>(&fun), nullptr);
                return fetchAndCall(vm, *fun, MakeIndexSequence<FunctionTraits<Fun>::arity>());
            }

            template <class Fun, size_t... ArgIndices>
            static auto fetchAndCall(HSQUIRRELVM vm, Fun fun, IndexSequence<ArgIndices...>)
                -> std::enable_if_t<std::is_same<ReturnType<Fun>, void>::value, SQInteger>
            {
                fun(
                    cpp(
                        getValue<SqType<ArgumentType<Fun, ArgIndices>>>(vm, ArgIndices + 2)
                        )...
                    );
                return 0;
            }

            template <class Fun, size_t... ArgIndices>
            static auto fetchAndCall(HSQUIRRELVM vm, Fun fun, IndexSequence<ArgIndices...>)
                -> std::enable_if_t<!std::is_same<ReturnType<Fun>, void>::value, SQInteger>
            {
                pushValue(
                    vm, 
                    sq(
                        fun(
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