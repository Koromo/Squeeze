#ifndef SQUEEZE_SQZDEF_H
#define SQUEEZE_SQZDEF_H

#include "squirrel/squirrel.h"
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

        /// function defined in Cpp
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

    /** Convert types of Cpp to the Squirrel types */
    template <class Cpp> struct ToSquirrel;
    template <> struct ToSquirrel<int> { using Type = SQInteger; };
    template <> struct ToSquirrel<size_t> { using Type = SQInteger; };
    template <> struct ToSquirrel<double> { using Type = SQFloat; };
    template <> struct ToSquirrel<float> { using Type = SQFloat; };
    template <> struct ToSquirrel<bool> { using Type = SQBool; };
    template <> struct ToSquirrel<const SQChar*> { using Type = const SQChar*; };
    template <> struct ToSquirrel<string_t> { using Type = const SQChar*; };

    template <class Cpp>
    using SqType = typename ToSquirrel<std::remove_const_t<std::remove_reference_t<Cpp>>>::Type;

    /** Convert the value of Cpp to a Squirrel usable value. */
    template <class T>
    auto sq(T val) -> SqType<T>
    {
        return static_cast<SqType<T>>(val);
    }

    inline const string_t& sq(const string_t& val)
    {
        return val;
    }
}

#endif
