#ifndef SQUEEZE_SQZCLASS_H
#define SQUEEZE_SQZCLASS_H

#include "sqzclosure.h"
#include "sqztable.h"
#include "sqztableimpl.h"
#include "sqzstackop.h"
#include "sqzdef.h"
#include "squirrel/squirrel.h"
#include <type_traits>
#include <tuple>

namespace squeeze
{
    /** The Class object handle */
    template <class Class>
    class HClass : public HTableImpl
    {
    public:
        /** Construct */
        HClass() = default;

        /** Create a class object */
        explicit HClass(HVM vm)
        {
            vm_ = vm;
            const auto top = sq_gettop(vm_);
            sq_newclass(vm_, SQFalse);
            sq_getstackobj(vm_, -1, &obj_);
            sq_addref(vm_, &obj_);
            sq_settop(vm_, top);
        }

        /** Create a class object with extend 'base' class */
        template <class U>
        explicit HClass(HClass<U> base)
        {
            vm_ = base.vm();
            const auto top = sq_gettop(vm_);
            pushValue(vm_, base);
            sq_newclass(vm_, SQTrue);
            sq_getstackobj(vm_, -1, &obj_);
            sq_addref(vm_, &obj_);
            sq_settop(vm_, top);
        }

        /** Add constructor */
        template <class... Args>
        HClass& ctor()
        {
            newClosure(SQZ_T("constructor"), CtorClosure<Class, std::tuple<Args...>>::ctor, false);
            return *this;
        }

        /** Add a member as a variable */
        template <class T>
        HClass& var(const string_t& name, const T& val, bool isStatic = false)
        {
            newSlot(name, val, isStatic);
            return *this;
        }

        /** Add a member as a table */
        HClass& table(const string_t& name, HTable table, bool isStatic = false)
        {
            newSlot(name, table, isStatic);
            return *this;
        }

        /** Add a member as a non-static function */
        template <class Return, class... Args>
        HClass& fun(const string_t& name, Return(Class::*fun)(Args...))
        {
            newClosure(name, MemClosure<Class, decltype(fun)>::fun, false, fun);
            return *this;
        }

        /** Add a new slot as a non-static function. The embedded function returns a class instance. */
        template <class Return, class... Args>
        HTable& fun(const string_t& key, Return(Class::*fun)(Args...), const string_t& retClassKey)
        {
            MemoryBlock<const SQChar*> mem;
            mem.p = retClassKey.data();
            mem.s = (retClassKey.length() + 1) * sizeof(SQChar);
            newClosure(name, MemClosure<Class, decltype(fun)>::fun, false, mem, fun);
            return *this;
        }

        /** Add a member as a static function */
        template <class Fun, class = std::enable_if_t<!std::is_member_function_pointer<Fun>::value>>
        HClass& fun(const string_t& name, Fun fun)
        {
            newClosure(name, Closure<Fun>::fun, true, fun);
            return *this;
        }

        /** Add a new slot as a static function. The embedded function returns a class instance. */
        template <class Fun>
        HTable& fun(const string_t& key, Fun fun, const string_t& retClassKey)
        {
            MemoryBlock mem;
            mem.p = retClassKey.data();
            mem.s = (retClassKey.length() + 1) * sizeof(SQChar);
            newClosure(key, Closure<Fun>::fun, true, fun, mem);
            return *this;
        }
    };
}

#endif
