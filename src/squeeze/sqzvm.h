#ifndef SQUEEZE_SQZVM_H
#define SQUEEZE_SQZVM_H

#include "sqzdef.h"
#include "../squirrel/squirrel.h"

namespace squeeze
{
    class HTable;

    /// The VM
    class HVM
    {
    private:
        HSQUIRRELVM vm_;

    public:
        /// Constructor
        HVM() = default;

        /// Copy
        HVM(const HVM&) = default;

        /// Mode
        HVM(HVM&&) = default;

        /// Copy
        HVM& operator=(const HVM&) = default;

        /// Mode
        HVM& operator=(HVM&&) = default;

        /// Cast to HSQUIRRELVM
        operator HSQUIRRELVM()
        {
            return vm_;
        }

        /// Open a new VM
        void open(size_t stackSize)
        {
            vm_ = sq_open(stackSize);
        }

        /// Close the VM
        void close()
        {
            sq_close(vm_);
        }

        /// Return the current root table
        HTable rootTable();

        /// Set root table
        void setRootTable(HTable root);
    };
}

#endif

