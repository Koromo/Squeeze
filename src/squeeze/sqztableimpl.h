#ifndef SQUEEZE_SQZTABLEIMPL_H
#define SQUEEZE_SQZTABLEIMPL_H

#include "sqzobject.h"
#include "sqzstackop.h"
#include "sqzdef.h"
#include "squirrel/squirrel.h"
#include <type_traits>

namespace squeeze
{
    /** The basic implementation of table object handlers */
    class HTableImpl : public HObject
    {
    public:
        /**
        Whether the object type mapped by 'key' is same to 'type' or not. 
        This function returns false if an object mapped by 'key' is not exist.
        */
        bool is(ObjectType type, const string_t& key)
        {
            bool isSame = false;

            const auto top = sq_gettop(vm_);
            pushValue(vm_, obj_, key);
            if (SQ_SUCCEEDED(sq_get(vm_, -2)))
            {
                isSame = sq_gettype(vm_, -1) == static_cast<SQObjectType>(type);
            }
            sq_settop(vm_, top);

            return isSame;
        }

    protected:
        template <class T>
        void newSlot(const string_t& key, const T& val, bool bstatic)
        {
            const auto top = sq_gettop(vm_);
            pushValue(vm_, obj_, key, val);
            if (SQ_FAILED(sq_newslot(vm_, -3, bstatic)))
            {
                sq_settop(vm_, top);
                failed<ObjectHandlingFailed>(vm_, "sq_newslot() failed.");
            }
            sq_settop(vm_, top);
        }

        template <class... FreeVars>
        void newClosure(const string_t& key, SQFUNCTION closure, bool bstatic, const FreeVars&... freeVars)
        {
            const auto top = sq_gettop(vm_);
            pushValue(vm_, obj_, key);
            pushUserData(vm_, freeVars...);
            sq_newclosure(vm_, closure, sizeof...(FreeVars));
            if (SQ_FAILED(sq_newslot(vm_, -3, bstatic)))
            {
                sq_settop(vm_, top);
                failed<ObjectHandlingFailed>(vm_, "sq_newclosure() failed.");
            }
            sq_settop(vm_, top);
        }

        template <class Return, class... Args>
        auto call(const string_t& key, HSQOBJECT env, const Args&... args)
            -> std::enable_if_t<std::is_void<Return>::value, void>
        {
            const auto top = sq_gettop(vm_);
            if (!prepareCall(key, env, args...))
            {
                sq_settop(vm_, top);
                failed<CallFailed>(vm_, "sq_call() failed.");
            }
            if (SQ_FAILED(sq_call(vm_, sizeof...(Args)+1, SQFalse, SQTrue)))
            {
                sq_settop(vm_, top);
                failed<CallFailed>(vm_, "sq_call() failed.");
            }
            sq_settop(vm_, top);
        }

        template <class Return, class... Args>
        auto call(const string_t& key, HSQOBJECT env, const Args&... args)
            -> std::enable_if_t<!std::is_void<Return>::value, Return>
        {
            const auto top = sq_gettop(vm_);
            if (!prepareCall(key, env, args...))
            {
                sq_settop(vm_, top);
                failed<CallFailed>(vm_, "sq_call() failed.");
            }
            if (SQ_FAILED(sq_call(vm_, sizeof...(Args)+1, SQTrue, SQTrue)))
            {
                sq_settop(vm_, top);
                failed<CallFailed>(vm_, "sq_call() failed.");
            }
            const auto ret = getValue<Return>(vm_, -1);
            sq_settop(vm_, top);
            return ret;
        }

    private:
        template <class... Args>
        bool prepareCall(const string_t& key, HSQOBJECT env, const Args&... args)
        {
            pushValue(vm_, obj_, key);
            if (SQ_FAILED(sq_get(vm_, -2)))
            {
                return false;
            }
            pushValue(vm_, env, args...);
            return true;
        }
    };
}

#endif
