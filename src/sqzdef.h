#ifndef SQUEEZE_SQZDEF_H
#define SQUEEZE_SQZDEF_H

#include <squirrel.h>
#include <string>
#include <tuple>
#include <type_traits>

/** Replace a string literal to the used character set. */
#define SQZ_T(s) _SC(s)

namespace squeeze
{
    /** The string type */
    using string_t = std::basic_string<SQChar>;

    /** Object types */
    enum class ObjectType
    {
        Null = OT_NULL,
        Integer = OT_INTEGER,
        Real = OT_FLOAT,
        Bool = OT_BOOL,
        String = OT_STRING,
        Table = OT_TABLE,
        Class = OT_CLASS,

        /// function defined in Squirrel
        Function = OT_CLOSURE, 

        /// function defined in C++
        HostFunction = OT_NATIVECLOSURE, 
    };

    /** The exception of stack operations */
    class StackOperationFailed : public std::runtime_error
    {
    public:
        explicit StackOperationFailed(const std::string& msg = "stack operations exception")
            : std::runtime_error(msg) {}
    };

    /** The exception of object handlings */
    class ObjectHandlingFailed : public std::runtime_error
    {
    public:
        explicit ObjectHandlingFailed(const std::string& msg = "object handlings exception")
            : std::runtime_error(msg) {}
    };

    /** The exception of calls */
    class CallFailed : public std::runtime_error
    {
    public:
        explicit CallFailed(const std::string& msg = "calls exception")
            : std::runtime_error(msg) {}
    };

    /** The script exception */
    class ScriptException : public std::runtime_error
    {
    public:
        explicit ScriptException(const std::string& msg = "script exception")
            : std::runtime_error(msg) {}
    };

    /** The class converter */
    template <class T>
    struct ClassConv
    {
        T v;
        string_t classKey;
    };

    /** Defined as U type if T is a integer type. */
    template <class T, class U = void, class X = std::remove_cv_t<std::remove_reference_t<T>>>
    using EnableInteger = std::enable_if_t<std::is_integral<X>::value && !std::is_same<X, bool>::value, U>;

    /** Defined as U type if T is a boolean type. */
    template <class T, class U = void, class X = std::remove_cv_t<std::remove_reference_t<T>>>
    using EnableBool = std::enable_if_t<std::is_same<X, bool>::value, U>;

    /** Defined as U type if T is a boolean type. */
    template <class T, class U = void, class X = std::remove_cv_t<std::remove_reference_t<T>>>
    using EnableReal = std::enable_if_t<std::is_floating_point<X>::value, U>;

    /** Defined as U type if T is a character array. */
    template <class T, class U = void, class X = std::remove_cv_t<std::remove_reference_t<T>>>
    using EnableChars = std::enable_if_t<std::is_same<X, const SQChar*>::value, U>;

    /** Defined as U type if T is a string type. */
    template <class T, class U = void, class X = std::remove_cv_t<std::remove_reference_t<T>>>
    using EnableString = std::enable_if_t<std::is_same<X, string_t>::value, U>;
}

#endif
