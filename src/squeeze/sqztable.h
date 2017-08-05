#ifndef SQUEEZE_SQZTABLE_H
#define SQUEEZE_SQZTABLE_H

#include "sqzobject.h"
#include "sqzstackop.h"
#include "sqzdef.h"
#include "sqztraits.h"
#include "../squirrel/squirrel.h"
#include <type_traits>

namespace squeeze
{
    /// The Table
    class HTable : public HObject
    {
    public:
        /// Create a table
        explicit HTable(HVM vm)
            : HObject(vm)
        {
            const auto top = sq_gettop(vm);
            sq_newtable(vm);
            sq_getstackobj(vm, -1, &obj_);
            sq_addref(vm, &obj_);
            sq_settop(vm, top);
        }

        HTable(HVM vm, HSQOBJECT obj)
            : HObject(vm)
        {
            obj_ = obj;
        }

        /// Create a clone
        HTable clone()
        {
            HTable clo(vm_);

            const auto top = sq_gettop(vm_);
            sq_pushobject(vm_, obj_);
            sq_pushobject(vm_, clo.obj_);
            sq_pushnull(vm_);
            while (SQ_SUCCEEDED(sq_next(vm_, -3)))
            {
                sq_newslot(vm_, -4, SQFalse);
            }
            sq_settop(vm_, top);

            return clo;
        }

        /// Set a variable
        template <class T>
        HTable& setVariable(const char_t* name, T val)
        {
            const auto top = sq_gettop(vm_);
            sq_pushobject(vm_, obj_);
            sq_pushstring(vm_, name, -1);
            pushValue(vm_, val);
            sq_newslot(vm_, -3, SQFalse);
            sq_settop(vm_, top);
            return *this;
        }

        /// Set a function as a closure
        template <class Fun>
        HTable& setFunction(const char_t* name, Fun fun)
        {
            setClosure(name, Closure::function<Fun>, fun);
            return *this;
        }

        /// Set a table
        HTable& setTable(const char_t* name, HTable t)
        {
            const auto top = sq_gettop(vm_);
            sq_pushobject(vm_, obj_);
            sq_pushstring(vm_, name, -1);
            sq_pushobject(vm_, t);
            sq_newslot(vm_, -3, SQFalse);
            sq_settop(vm_, top);
            return *this;
        }

        /// Check the value type
        bool is(TypeTag type, const char_t* name)
        {
            bool isSame = false;

            const auto top = sq_gettop(vm_);
            sq_pushobject(vm_, obj_);
            sq_pushstring(vm_, name, -1);
            if (SQ_SUCCEEDED(sq_get(vm_, -2)))
            {
                isSame = sq_gettype(vm_, -1) == static_cast<SQObjectType>(type);
            }
            sq_settop(vm_, top);

            return isSame;
        }

    private:
        struct Closure
        {
            template <class Fun>
            static SQInteger function(HSQUIRRELVM vm)
            {
                Fun* fun;
                auto r = sq_getuserdata(vm, -1, reinterpret_cast<SQUserPointer*>(&fun), nullptr);
                return function(vm, *fun, makeIndexSequence<ArgumentTypes<Fun>::length>());
            }

            template <class Fun, size_t... ArgIndices>
            static auto function(HSQUIRRELVM vm, Fun fun, IndexSequence<ArgIndices...>)
                -> std::enable_if_t<std::is_same<ResultType<Fun>, void>::value, SQInteger>
            {
                using Args = ArgumentTypes<Fun>;
                fun(getValue<Args::Type<ArgIndices>>(vm, ArgIndices - Args::length)...);
                return 0;
            }

            template <class Fun, size_t... ArgIndices>
            static auto function(HSQUIRRELVM vm, Fun fun, IndexSequence<ArgIndices...>)
                -> std::enable_if_t<!std::is_same<ResultType<Fun>, void>::value, SQInteger>
            {
                pushValue(vm, fun(getValue<Args::Type<ArgIndices>>(vm, ArgIndices - Args::length)...));
                return 1;
            }
        };

        template <class... FreeVars>
        void setClosure(const char_t* name, SQFUNCTION closure, FreeVars... freeVars)
        {
            const auto top = sq_gettop(vm_);
            sq_pushobject(vm_, obj_);
            sq_pushstring(vm_, name, -1);
            pushAsUserData(vm_, freeVars...);
            sq_newclosure(vm_, closure, sizeof...(FreeVars));
            sq_newslot(vm_, -3, SQFalse);
            sq_settop(vm_, top);
        }
    };
}

#endif