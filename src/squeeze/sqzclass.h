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
    private:
        HTable setTable_;
        HTable getTable_;

    public:
        /** Construct */
        HClass() = default;

        /** Create a class object */
        explicit HClass(HVM vm)
            : setTable_(vm)
            , getTable_(vm)
        {
            vm_ = vm;
            const auto top = sq_gettop(vm_);
            sq_newclass(vm_, SQFalse);
            sq_getstackobj(vm_, -1, &obj_);
            sq_addref(vm_, &obj_);
            sq_settop(vm_, top);
            init();
        }

        /** Create a class object with extend 'base' class. */
        template <class U, class = std::enable_if_t<std::is_base_of<U, Class>::value>>
        explicit HClass(HClass<U> base)
            : setTable_(base.setTable_.clone())
            , getTable_(base.getTable_.clone())
        {
            vm_ = base.vm();
            const auto top = sq_gettop(vm_);
            pushValue(vm_, base);
            sq_newclass(vm_, SQTrue);
            sq_getstackobj(vm_, -1, &obj_);
            sq_addref(vm_, &obj_);
            sq_settop(vm_, top);
            init();
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

        /** Add a member as a setter. */
        template <
            class Setter,
            class Traits = FunctionTraits<Setter>,
            class = std::enable_if_t<std::is_same<typename Traits::ClassType, Class>::value>,
            class = std::enable_if_t<std::is_same<typename Traits::ReturnType, void>::value>,
            class = std::enable_if_t<(Traits::arity == 1)>
        >
        HClass& setter(const string_t& name, Setter set)
        {
            setTable_.newClosure(name, MemClosure<Class, Setter>::fun, false, set);
            return *this;
        }

        /** Add a member as a getter. */
        template <
            class Getter,
            class Traits = FunctionTraits<Getter>,
            class = std::enable_if_t<std::is_same<typename Traits::ClassType, Class>::value>,
            class = std::enable_if_t<!std::is_same<typename Traits::ReturnType, void>::value>,
            class = std::enable_if_t<(Traits::arity == 0)>
        >
        HClass& getter(const string_t& name, Getter get)
        {
            getTable_.newClosure(name, MemClosure<Class, Getter>::fun, false, get);
            return *this;
        }

        /** Add a member as a property. */
        template <
            class Getter, class Setter,
            class R = ReturnType<Getter>,
            class A = ArgumentType<Setter, 0>,
            class = std::enable_if_t<std::is_same<R, A>::value>
        >
        HClass& prop(const string_t& name, Getter get, Setter set)
        {
            getter(name, get);
            setter(name, set);
            return *this;
        }

        /** Add a member as a non-static function */
        template <
            class Fun,
            class = std::enable_if_t<std::is_same<typename FunctionTraits<Fun>::ClassType, Class>::value>,
            class = std::enable_if_t<!std::is_class<ReturnType<Fun>>::value>
        >
        HClass& fun(const string_t& name, Fun fun)
        {
            newClosure(name, MemClosure<Class, Fun>::fun, false, fun);
            return *this;
        }

        /// ditto
        template <
            class Fun,
            class = std::enable_if_t<std::is_same<typename FunctionTraits<Fun>::ClassType, Class>::value>,
            class = std::enable_if_t<std::is_class<ReturnType<Fun>>::value>
        >
        HClass& fun(const string_t& name, Fun fun, const string_t& retClassKey)
        {
            MemoryBlock<const SQChar*> mem;
            mem.p = retClassKey.data();
            mem.s = (retClassKey.length() + 1) * sizeof(SQChar);
            newClosure(name, MemClosure<Class, Fun>::fun, false, mem, fun);
            return *this;
        }

        /** Add a member as a static function */
        template <
            class Fun,
            class = std::enable_if_t<!std::is_member_function_pointer<Fun>::value>,
            class = std::enable_if_t<!std::is_class<ReturnType<Fun>>::value>
        >
        HClass& staticFun(const string_t& name, Fun fun)
        {
            newClosure(name, Closure<Fun>::fun, true, fun);
            return *this;
        }

        /// ditto
        template <
            class Fun,
            class = std::enable_if_t<!std::is_member_function_pointer<Fun>::value>,
            class = std::enable_if_t<std::is_class<ReturnType<Fun>>::value>
        >
        HClass& staticFun(const string_t& name, Fun fun, const string_t& retClassKey)
        {
            MemoryBlock mem;
            mem.p = retClassKey.data();
            mem.s = (retClassKey.length() + 1) * sizeof(SQChar);
            newClosure(name, Closure<Fun>::fun, true, fun, mem);
            return *this;
        }

    private:
        void init()
        {
            const auto top = sq_gettop(vm_);
            pushValue(vm_, obj_);

            pushValue(vm_, SQZ_T("_set"), setTable_);
            sq_newclosure(vm_, MemClosure<Class>::opSet, 1);
            if (SQ_FAILED(sq_newslot(vm_, -3, false)))
            {
                sq_settop(vm_, top);
                failed<ObjectHandlingFailed>(vm_, "sq_newslot() failed.");
            }

            pushValue(vm_, SQZ_T("_get"), getTable_);
            sq_newclosure(vm_, MemClosure<Class>::opGet, 1);
            if (SQ_FAILED(sq_newslot(vm_, -3, false)))
            {
                sq_settop(vm_, top);
                failed<ObjectHandlingFailed>(vm_, "sq_newslot() failed.");
            }

            sq_settop(vm_, top);
        }
    };
}

#endif
