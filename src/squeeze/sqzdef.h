#ifndef SQUEEZE_SQZDEF_H
#define SQUEEZE_SQZDEF_H

#include "squirrel/squirrel.h"
#include <tuple>
#include <exception>
#include <stdexcept>

#define SQZ_T(s) _SC(s)

namespace squeeze
{
    /// Real number type and string type.
#ifdef SQZ_DOUBLE
    using real_t = double;
#else
    using real_t = float;
#endif

#ifdef _UNICODE
    using string_t = const wchar_t*;
#else
    using string_t = const char*;
#endif

    /// Type tags
    enum class TypeTag
    {
        Null = OT_NULL,
        Integer = OT_INTEGER,
        Float = OT_FLOAT,
        Bool = OT_BOOL,
        String = OT_STRING,
        Table = OT_TABLE,
        Closure = OT_NATIVECLOSURE,
        Class = OT_CLASS
    };

    template <class inCpp> struct TypeInSquirrel { using Type = inCpp; };
    template <class inSq> struct TypeInCpp { using Type = inSq; };

    template <> struct TypeInSquirrel<int> { using Type = SQInteger; };
    template <> struct TypeInCpp<SQInteger> { using Type = int; };

    template <> struct TypeInSquirrel<real_t> { using Type = SQFloat; };
    template <> struct TypeInCpp<SQFloat> { using Type = real_t; };

    template <> struct TypeInSquirrel<bool> { using Type = SQBool; };
    template <> struct TypeInCpp<SQBool> { using Type = bool; };

    template <> struct TypeInSquirrel<string_t> { using Type = const SQChar*; };
    template <> struct TypeInCpp<const SQChar*> { using Type = string_t; };

    template <class inCpp>
    using SqType = typename TypeInSquirrel<inCpp>::Type;

    template <class inSq>
    using CppType = typename TypeInCpp<inSq>::Type;

    template <class T, class U = SqType<T>>
    U sq(T val)
    {
        return static_cast<U>(val);
    }

    template <class T, class U = CppType<T>>
    U cpp(T val)
    {
        return static_cast<U>(val);
    }
}

#endif
