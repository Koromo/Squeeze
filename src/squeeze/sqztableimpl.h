#ifndef SQUEEZE_SQZTABLEIMPL_H
#define SQUEEZE_SQZTABLEIMPL_H

#include "sqzobject.h"
#include "sqzstackop.h"
#include "sqzdef.h"
#include "squirrel/squirrel.h"
#include <type_traits>

namespace squeeze
{
    /// The basic implementation of tables
    class HTableImpl : public HObject
    {
    public:
        /// Check the value type
        bool is(TypeTag type, string_t name)
        {
            bool isSame = false;

            const auto top = sq_gettop(vm_);
            pushValue(vm_, obj_, sq(name));
            if (SQ_SUCCEEDED(sq_get(vm_, -2)))
            {
                isSame = sq_gettype(vm_, -1) == static_cast<SQObjectType>(type);
            }
            sq_settop(vm_, top);

            return isSame;
        }

    protected:
        template <class T>
        void newSlot(string_t name, T val, bool bstatic)
        {
            const auto top = sq_gettop(vm_);
            pushValue(vm_, obj_, sq(name), sq(val));
            sq_newslot(vm_, -3, sq(bstatic));
            sq_settop(vm_, top);
        }

        template <class... FreeVars>
        void newClosure(string_t name, SQFUNCTION closure, bool bstatic, FreeVars... freeVars)
        {
            const auto top = sq_gettop(vm_);
            pushValue(vm_, obj_, sq(name));
            pushUserData(vm_, freeVars...);
            sq_newclosure(vm_, closure, sizeof...(FreeVars));
            sq_newslot(vm_, -3, sq(bstatic));
            sq_settop(vm_, top);
        }

        template <class Result, class... Args>
        auto call(string_t name, HSQOBJECT env, Args&&... args)
            -> std::enable_if_t<std::is_same<Result, void>::value, void>
        {
            const auto top = sq_gettop(vm_);
            if (!prepareCall(name, env, std::forward<Args>(args)...))
            {
                sq_settop(vm_, top);
                throw std::runtime_error("Failed to call function.");
            }
            sq_call(vm_, sizeof...(Args) + 1, SQFalse, SQTrue);
            sq_settop(vm_);
        }

        template <class Result, class... Args>
        auto call(string_t name, HSQOBJECT env, Args&&... args)
            -> std::enable_if_t<!std::is_same<Result, void>::value, Result>
        {
            const auto top = sq_gettop(vm_);
            if (!prepareCall(name, env, std::forward<Args>(args)...))
            {
                sq_settop(vm_, top);
                throw std::runtime_error("Failed to call function.");
            }
            sq_call(vm_, sizeof...(Args) + 1, SQTrue, SQTrue);
            const auto ret = getValue<SqType<Result>>(vm_, -1);
            sq_settop(vm_);
            return cpp(ret);
        }

    private:
        template <class... Args>
        bool prepareCall(string_t name, HSQOBJECT env, Args&&... args)
        {
            pushValue(vm_, obj_, sq(name));
            if (SQ_FAILED(sq_get(vm_, -2)))
            {
                return false;
            }
            pushValue(vm_, env, sq(std::forward<Args>(args))...);
            return true;
        }
    };
}

#endif
