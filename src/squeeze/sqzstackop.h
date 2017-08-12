#ifndef SQUEEZE_SQZSTACKOP_H
#define SQUEEZE_SQZSTACKOP_H

#include "sqzdef.h"
#include "sqzutil.h"
#include "squirrel/squirrel.h"
#include <stdexcept>
#include <string>
#include <cstring>

namespace squeeze
{
    /** Push the values into the stack. */
    template <class... Ts>
    void pushValue(HSQUIRRELVM vm, const SQChar* val, Ts... values)
    {
        sq_pushstring(vm, val, -1);
        pushValue(vm, values...);
    }

    /// ditto
    template <class... Ts>
    void pushValue(HSQUIRRELVM vm, const string_t& s, Ts... values)
    {
        sq_pushstring(vm, s.c_str(), s.length());
        pushValue(vm, values...);
    }

    /// ditto
    template <class... Ts>
    void pushValue(HSQUIRRELVM vm, SQInteger val, Ts... values)
    {
        sq_pushinteger(vm, val);
        pushValue(vm, values...);
    }

    /// ditto
    template <class... Ts>
    void pushValue(HSQUIRRELVM vm, SQFloat val, Ts... values)
    {
        sq_pushfloat(vm, val);
        pushValue(vm, values...);
    }

    /// ditto
    template <class... Ts>
    void pushValue(HSQUIRRELVM vm, SQBool val, Ts... values)
    {
        sq_pushbool(vm, val);
        pushValue(vm, values...);
    }

    /// ditto
    template <class... Ts>
    void pushValue(HSQUIRRELVM vm, std::nullptr_t, Ts... values)
    {
        sq_pushnull(vm);
        pushValue(vm, values...);
    }

    /// ditto
    template <class... Ts>
    void pushValue(HSQUIRRELVM vm, HSQOBJECT val, Ts... values)
    {
        sq_pushobject(vm, val);
        pushValue(vm, values...);
    }

    /// ditto
    inline void pushValue(HSQUIRRELVM vm)
    {
    }

    /** Get a value from the stack */
    template <class T>
    T getValue(HSQUIRRELVM vm, int id);

    /// ditto
    template <>
    inline const SQChar* getValue(HSQUIRRELVM vm, int id)
    {
        const SQChar* val;
        const auto sqr = sq_getstring(vm, id, &val);
        if (SQ_FAILED(sqr))
        {
            failed<StackOperationFailed>(vm, "sq_getstring() failed.");
        }
        return val;
    }

    /// ditto
    template <>
    inline string_t getValue(HSQUIRRELVM vm, int id)
    {
        return getValue<const SQChar*>(vm, id);
    }

    /// ditto
    template <>
    inline SQInteger getValue(HSQUIRRELVM vm, int id)
    {
        SQInteger val;
        const auto sqr = sq_getinteger(vm, id, &val);
        if (SQ_FAILED(sqr))
        {
            failed<StackOperationFailed>(vm, "sq_getinteger() failed.");
        }
        return val;
    }

    /// ditto
    template <>
    inline SQFloat getValue(HSQUIRRELVM vm, int id)
    {
        SQFloat val;
        const auto sqr = sq_getfloat(vm, id, &val);
        if (SQ_FAILED(sqr))
        {
            failed<StackOperationFailed>(vm, "sq_getfloat() failed.");
        }
        return val;
    }

    /// ditto
    template <>
    inline SQBool getValue(HSQUIRRELVM vm, int id)
    {
        SQBool val;
        const auto sqr = sq_getbool(vm, id, &val);
        if (SQ_FAILED(sqr))
        {
            failed<StackOperationFailed>(vm, "sq_getbool() failed.");
        }
        return val;
    }

    /** Push the values as user datas */
    template <class T, class... Ts>
    void pushUserData(HSQUIRRELVM vm, T& val, Ts... values)
    {
        const auto usrData = sq_newuserdata(vm, sizeof(val));
        std::memcpy(usrData, &val, sizeof(val));
        pushUserData(vm, values...);
    }

    /// ditto
    template <class T, class... Ts>
    void pushUserData(HSQUIRRELVM vm, T&& val, Ts... values)
    {
        pushUserData(vm, val, values...);
    }

    /// ditto
    inline void pushUserData(HSQUIRRELVM vm)
    {
    }
}

#endif
