#ifndef SQUEEZE_SQZTABLE_H
#define SQUEEZE_SQZTABLE_H

#include "sqztableimpl.h"
#include "sqzstackop.h"
#include "sqzdef.h"
#include "sqzutil.h"
#include "squirrel/squirrel.h"
#include <type_traits>

namespace squeeze
{
    template <class Class> class HClass;

    /** The Table object handle */
    class HTable : public HTableImpl
    {
    public:
        /** Construct */
        HTable() = default;

        /** Create a table object */
        explicit HTable(HVM vm)
        {
            vm_ = vm;
            const auto top = sq_gettop(vm_);
            sq_newtable(vm_);
            sq_getstackobj(vm_, -1, &obj_);
            sq_addref(vm_, &obj_);
            sq_settop(vm_, top);
        }

        /** Create with copy the object handle */
        HTable(HVM vm, HSQOBJECT obj)
        {
            vm_ = vm;
            obj_ = obj;
            sq_addref(vm_, &obj_);
        }

        /** Create a clone table */
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

        /** Add a new slot as a variable. */
        template <class T>
        HTable& var(const string_t& key, const T& val)
        {
            newSlot(key, val, false);
            return *this;
        }

        /** Add a new slot as a table. */
        HTable& table(const string_t& key, HTable table)
        {
            newSlot(key, table, false);
            return *this;
        }

        /** Add a new slot as a class. */
        template <class Class>
        HTable& klass(const string_t& key, HClass<Class> c);

        /** Add a new slot as a function. */
        template <class Fun>
        HTable& fun(const string_t& key, Fun fun)
        {
            newClosure(key, Closure::fun<Fun>, false, fun);
            return *this;
        }

        /** Call a function mapped by 'key'. */
        template <class Return, class... Args>
        Return call(const string_t& key, HTable env, const Args&... args)
        {
            return HTableImpl::call<Return>(key, env, args...);
        }

        // Referenced from HTable class and HClass class
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
                -> std::enable_if_t<std::is_void<ReturnType<Fun>>::value, SQInteger>
            {
                fun(getValue<ArgumentType<Fun, ArgIndices>>(vm, ArgIndices + 2)...);
                return 0;
            }

            template <class Fun, size_t... ArgIndices>
            static auto fetchAndCall(HSQUIRRELVM vm, Fun fun, IndexSequence<ArgIndices...>)
                -> std::enable_if_t<!std::is_void<ReturnType<Fun>>::value, SQInteger>
            {
                const auto ret = fun(getValue<ArgumentType<Fun, ArgIndices>>(vm, ArgIndices + 2)...);
                pushValue(vm, ret);
                return 1;
            }
        };
    };
}

#endif