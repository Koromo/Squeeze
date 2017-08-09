#ifndef SQUEEZE_SQZSTACKOP_H
#define SQUEEZE_SQZSTACKOP_H

#include "sqzdef.h"
#include "squirrel/squirrel.h"
#include <cstring>
#include <cassert>

namespace squeeze
{
    class StackException : public std::runtime_error
    {
    public:
        explicit StackException(const std::string& msg = "stack exception")
            : std::runtime_error(msg) {}
    };

    /// Push the values into the stack
    template <class... Ts>
    void pushValue(HSQUIRRELVM vm, const SQChar* s, Ts... values)
    {
        sq_pushstring(vm, s, -1);
        pushValue(vm, values...);
    }

    /// ditto
    template <class... Ts>
    void pushValue(HSQUIRRELVM vm, SQInteger i, Ts... values)
    {
        sq_pushinteger(vm, i);
        pushValue(vm, values...);
    }

    /// ditto
    template <class... Ts>
    void pushValue(HSQUIRRELVM vm, SQFloat f, Ts... values)
    {
        sq_pushfloat(vm, f);
        pushValue(vm, values...);
    }

    /// ditto
    template <class... Ts>
    void pushValue(HSQUIRRELVM vm, SQBool b, Ts... values)
    {
        sq_pushbool(vm, b);
        pushValue(vm, values...);
    }

    template <class... Ts>
    void pushValue(HSQUIRRELVM vm, std::nullptr_t, Ts... values)
    {
        sq_pushnull(vm);
        pushValue(vm, values...);
    }

    /// ditto
    template <class... Ts>
    void pushValue(HSQUIRRELVM vm, HSQOBJECT o, Ts... values)
    {
        sq_pushobject(vm, o);
        pushValue(vm, values...);
    }

    /// dito
    inline void pushValue(HSQUIRRELVM vm)
    {
    }

    /// Get a value from the stack
    template <class T>
    T getValue(HSQUIRRELVM vm, int id);

    /// dito
    template <>
    inline const SQChar* getValue(HSQUIRRELVM vm, int id)
    {
        const SQChar* c;
        const auto sqr = sq_getstring(vm, id, &c);
        if (SQ_FAILED(sqr))
        {
            throw StackException();
        }
        return c;
    }

    /// dito
    template <>
    inline SQInteger getValue(HSQUIRRELVM vm, int id)
    {
        SQInteger i;
        const auto sqr = sq_getinteger(vm, id, &i);
        if (SQ_FAILED(sqr))
        {
            throw StackException();
        }
        return static_cast<int>(i);
    }

    /// dito
    template <>
    inline SQFloat getValue(HSQUIRRELVM vm, int id)
    {
        SQFloat f;
        const auto sqr = sq_getfloat(vm, id, &f);
        if (SQ_FAILED(sqr))
        {
            throw StackException();
        }
        return f;
    }

    /// dito
    template <>
    inline SQBool getValue(HSQUIRRELVM vm, int id)
    {
        SQBool b;
        const auto sqr = sq_getbool(vm, id, &b);
        if (SQ_FAILED(sqr))
        {
            throw StackException();
        }
        return !!b;
    }

    /// Push the values as user datas
    template <class T, class... Ts>
    void pushUserData(HSQUIRRELVM vm, T& val, Ts... values)
    {
        const auto usrData = sq_newuserdata(vm, sizeof(val));
        std::memcpy(usrData, &val, sizeof(val));
        pushUserData(vm, values...);
    }

    template <class T, class... Ts>
    void pushUserData(HSQUIRRELVM vm, T&& val, Ts... values)
    {
        pushUserData(vm, val, values...);
    }

    inline void pushUserData(HSQUIRRELVM vm)
    {
    }
}

#endif
