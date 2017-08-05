#ifndef SQUEEZE_SQZSTACKOP_H
#define SQUEEZE_SQZSTACKOP_H

#include "sqzdef.h"
#include "../squirrel/squirrel.h"
#include <cstring>
#include <cassert>

namespace squeeze
{
    /// Push the values into the stack
    template <class... Ts>
    void pushValue(HSQUIRRELVM vm, const char_t* s, Ts... values)
    {
        sq_pushstring(vm, s, -1);
        pushValue(vm, values...);
    }

    /// ditto
    template <class... Ts>
    void pushValue(HSQUIRRELVM vm, int n, Ts... values)
    {
        sq_pushinteger(vm, n);
        pushValue(vm, values...);
    }

    /// ditto
    template <class... Ts>
    void pushValue(HSQUIRRELVM vm, float f, Ts... values)
    {
        sq_pushfloat(vm, f);
        pushValue(vm, values...);
    }

    /// ditto
    template <class... Ts>
    void pushValue(HSQUIRRELVM vm, bool b, Ts... values)
    {
        sq_pushbool(vm, b);
        pushValue(vm, values...);
    }

    /// dito
    void pushValue(HSQUIRRELVM vm)
    {
    }

    /// Get a value from the stack
    template <class T>
    T getValue(HSQUIRRELVM vm, int id);

    /// dito
    template <>
    const char_t* getValue(HSQUIRRELVM vm, int id)
    {
        const char_t* c;
        const auto sqr = sq_getstring(vm, id, &c);
        if (SQ_FAILED(sqr))
        {
            throw std::runtime_error("Error at sq_getstring().");
        }
        return c;
    }

    /// dito
    template <>
    int getValue(HSQUIRRELVM vm, int id)
    {
        SQInteger i;
        const auto sqr = sq_getinteger(vm, id, &i);
        if (SQ_FAILED(sqr))
        {
            throw std::runtime_error("Error at sq_getinteger().");
        }
        return static_cast<int>(i);
    }

    /// dito
    template <>
    float getValue(HSQUIRRELVM vm, int id)
    {
        float f;
        const auto sqr = sq_getfloat(vm, id, &f);
        if (SQ_FAILED(sqr))
        {
            throw std::runtime_error("Error at sq_getfloat().");
        }
        return f;
    }

    /// dito
    template <>
    bool getValue(HSQUIRRELVM vm, int id)
    {
        SQBool b;
        const auto sqr = sq_getbool(vm, id, &b);
        if (SQ_FAILED(sqr))
        {
            throw std::runtime_error("Error at sq_getbool().");
        }
        return !!b;
    }

    /// Push the values as user datas
    template <class T, class... Ts>
    void pushAsUserData(HSQUIRRELVM vm, T& val, Ts... values)
    {
        const auto usrData = sq_newuserdata(vm, sizeof(val));
        std::memcpy(usrData, &val, sizeof(val));
        pushAsUserData(vm, values...);
    }

    template <class T, class... Ts>
    void pushAsUserData(HSQUIRRELVM vm, T&& val, Ts... values)
    {
        pushAsUserData(vm, val, values...);
    }

    inline void pushAsUserData(HSQUIRRELVM vm)
    {
    }
}

#endif
