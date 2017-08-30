#ifndef SQUEEZE_SQZSTACKOP_H
#define SQUEEZE_SQZSTACKOP_H

#include "sqzdef.h"
#include "sqzutil.h"
#include <squirrel.h>
#include <type_traits>
#include <cstring>

namespace squeeze
{
    /** Push the values into the stack. */
    template <class T, class... Ts>
    EnableInteger<T> pushValue(HSQUIRRELVM vm, T val, Ts&&... values)
    {
        const auto v = static_cast<SQInteger>(val);
        sq_pushinteger(vm, v);
        pushValue(vm, std::forward<Ts>(values)...);
    }

    /// ditto
    template <class T, class... Ts>
    EnableReal<T> pushValue(HSQUIRRELVM vm, T val, Ts&&... values)
    {
        const auto v = static_cast<SQFloat>(val);
        sq_pushfloat(vm, v);
        pushValue(vm, std::forward<Ts>(values)...);
    }

    /// ditto
    template <class T, class... Ts>
    EnableBool<T> pushValue(HSQUIRRELVM vm, T val, Ts&&... values)
    {
        const SQBool v = val;
        sq_pushbool(vm, v);
        pushValue(vm, std::forward<Ts>(values)...);
    }

    /// ditto
    template <class T, class... Ts>
    EnableChars<T> pushValue(HSQUIRRELVM vm, T val, Ts&&... values)
    {
        sq_pushstring(vm, val, -1);
        pushValue(vm, std::forward<Ts>(values)...);
    }

    /// ditto
    template <class T, class... Ts>
    EnableString<T> pushValue(HSQUIRRELVM vm, const T& val, Ts&&... values)
    {
        sq_pushstring(vm, val.c_str(), val.length());
        pushValue(vm, std::forward<Ts>(values)...);
    }

    /// ditto
    template <class T, class... Ts>
    auto pushValue(HSQUIRRELVM vm, T val, Ts&&... values)
        -> std::enable_if_t<std::is_same<T, std::nullptr_t>::value>
    {
        sq_pushnull(vm);
        pushValue(vm, std::forward<Ts>(values)...);
    }

    /// ditto
    template <class T, class... Ts>
    auto pushValue(HSQUIRRELVM vm, T val, Ts&&... values)
        -> std::enable_if_t<std::is_convertible<T, HSQOBJECT>::value>
    {
        sq_pushobject(vm, val);
        pushValue(vm, std::forward<Ts>(values)...);
    }

    struct UserData
    {
        const void* p;
        size_t s;
        UserData(const void* p_, size_t s_) : p(p_), s(s_) {}
    };

    /// ditto
    template <class... Ts>
    void pushValue(HSQUIRRELVM vm, const UserData& val, Ts... values)
    {
        const auto usrData = sq_newuserdata(vm, val.s);
        std::memcpy(usrData, val.p, val.s);
        pushValue(vm, values...);
    }

    /// ditto
    inline void pushValue(HSQUIRRELVM vm)
    {
    }

    /** Get a value from the stack */
    inline SQInteger getInteger(HSQUIRRELVM vm, int id)
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
    inline SQFloat getFloat(HSQUIRRELVM vm, int id)
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
    inline SQBool getBool(HSQUIRRELVM vm, int id)
    {
        SQBool val;
        const auto sqr = sq_getbool(vm, id, &val);
        if (SQ_FAILED(sqr))
        {
            failed<StackOperationFailed>(vm, "sq_getbool() failed.");
        }
        return val;
    }

    /// ditto
    inline const SQChar* getString(HSQUIRRELVM vm, int id)
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
    template <class T>
    EnableInteger<T, T> getValue(HSQUIRRELVM vm, int id)
    {
        const auto type = sq_gettype(vm, id);
        switch (type)
        {
        case OT_INTEGER: return static_cast<T>(getInteger(vm, id));
        case OT_FLOAT: return static_cast<T>(getFloat(vm, id));
        case OT_BOOL: return static_cast<T>(getBool(vm, id));
        default: failed<StackOperationFailed>(vm, "A type mismatching in getValue()");
        }
        return 0;
    }

    /// ditto
    template <class T>
    EnableReal<T, T> getValue(HSQUIRRELVM vm, int id)
    {
        const auto type = sq_gettype(vm, id);
        switch (type)
        {
        case OT_INTEGER: return static_cast<T>(getInteger(vm, id));
        case OT_FLOAT: return static_cast<T>(getFloat(vm, id));
        case OT_BOOL: return static_cast<T>(getBool(vm, id));
        default: failed<StackOperationFailed>(vm, "A type mismatching in getValue()");
        }
        return 0;
    }

    /// ditto
    template <class T>
    EnableBool<T, T> getValue(HSQUIRRELVM vm, int id)
    {
        const auto type = sq_gettype(vm, id);
        switch (type)
        {
        case OT_INTEGER: return static_cast<T>(!!getInteger(vm, id));
        case OT_BOOL: return static_cast<T>(!!getBool(vm, id));
        default: failed<StackOperationFailed>(vm, "A type mismatching in getValue()");
        }
        return 0;
    }

    /// ditto
    template <class T>
    EnableChars<T, T> getValue(HSQUIRRELVM vm, int id)
    {
        const auto type = sq_gettype(vm, id);
        switch (type)
        {
        case OT_STRING: return static_cast<T>(getString(vm, id));
        default: failed<StackOperationFailed>(vm, "A type mismatching in getValue()");
        }
        return nullptr;
    }

    /// ditto
    template <class T>
    EnableString<T, T> getValue(HSQUIRRELVM vm, int id)
    {
        return getValue<const SQChar*>(vm, id);
    }

    /** Push the values as user datas */
    template <class T, class... Ts>
    void pushUserData(HSQUIRRELVM vm, std::pair<const T*, size_t> val, std::pair<const Ts*, size_t>... values)
    {
        const auto usrData = sq_newuserdata(vm, val.second);
        std::memcpy(usrData, val.first, val.second);
        pushUserData(vm, values...);
    }

    /// ditto
    inline void pushUserData(HSQUIRRELVM vm)
    {
    }
}

#endif
