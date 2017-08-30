#ifndef SQUEEZE_SQZVM_H
#define SQUEEZE_SQZVM_H

#include "sqzdef.h"
#include <squirrel.h>
#include <sqstdio.h>
#include <sqstdblob.h>
#include <sqstdmath.h>
#include <sqstdsystem.h>
#include <sqstdstring.h>
#include <memory>

namespace squeeze
{
    class HTable;

    /** The VM handler */
    class HVM
    {
    private:
        std::shared_ptr<bool> valid_;
        HSQUIRRELVM vm_;

    public:
        /** Construct */
        HVM()
            : valid_(new bool(false))
            , vm_()
        {
        }

        /** Copy */
        HVM(const HVM&) = default;

        /** Move */
        HVM(HVM&&) = default;

        /** Copy */
        HVM& operator=(const HVM&) = default;

        /** Move */
        HVM& operator=(HVM&&) = default;

        /** Cast to HSQUIRRELVM */
        operator HSQUIRRELVM()
        {
            return vm_;
        }

        /** Whether the handled VM is valid or not */
        bool valid() const
        {
            return *valid_;
        }

        /** Register the std input/output library */
        void iolib()
        {
            sq_pushroottable(vm_);
            sqstd_register_iolib(vm_);
            sq_poptop(vm_);
        }

        /** Register the std blob library */
        void bloblib()
        {
            sq_pushroottable(vm_);
            sqstd_register_bloblib(vm_);
            sq_poptop(vm_);
        }

        /** Register the std math library */
        void mathlib()
        {
            sq_pushroottable(vm_);
            sqstd_register_mathlib(vm_);
            sq_poptop(vm_);
        }

        /** Register the std system library */
        void systemlib()
        {
            sq_pushroottable(vm_);
            sqstd_register_systemlib(vm_);
            sq_poptop(vm_);
        }

        /** Register the std string library */
        void stringlib()
        {
            sq_pushroottable(vm_);
            sqstd_register_stringlib(vm_);
            sq_poptop(vm_);
        }

        /** Open a new VM */
        void open(size_t stackSize)
        {
            vm_ = sq_open(stackSize);
            *valid_ = true;
        }

        /** Close the handled VM */
        void close()
        {
            sq_close(vm_);
            *valid_ = false;
        }

        /** Return the root table of the handled VM */
        HTable rootTable();

        /** Set a new root table to the handled VM */
        void setRootTable(HTable root);
    };
}

#endif

