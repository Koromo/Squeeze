#ifndef SQUEEZE_SQZOBJECT_H
#define SQUEEZE_SQZOBJECT_H

#include "sqzvm.h"
#include "sqzdef.h"
#include "squirrel/squirrel.h"

namespace squeeze
{
    /// The Object handle
    class HObject
    {
    protected:
        HVM vm_;
        HSQOBJECT obj_;

    public:
        /// Construct
        HObject()
            : vm_()
            , obj_()
        {
            sq_resetobject(&obj_);
        }

        /// Copy
        HObject(const HObject& that)
            : vm_()
            , obj_()
        {
            sq_resetobject(&obj_);
            *this = that;
        }

        /// Move
        HObject(HObject&& that)
            : vm_()
            , obj_()
        {
            sq_resetobject(&obj_);
            *this = that;
        }

        /// Destruct
        virtual ~HObject()
        {
            release();
        }

        /// Copy
        HObject& operator=(const HObject& that)
        {
            release();

            vm_ = that.vm_;
            obj_ = that.obj_;
            if (!sq_isnull(obj_))
            {
                sq_addref(vm_, &obj_);
            }
            return *this;
        }

        /// Move
        HObject& operator=(HObject&& that)
        {
            vm_ = that.vm_;
            obj_ = that.obj_;
            that.release();
            return *this;
        }

        /// Cast to HSQOBJECT
        operator HSQOBJECT()
        {
            return obj_;
        }

        /// Return the VM
        HVM vm()
        {
            return vm_;
        }

        /// Release the object handle
        void release()
        {
            /// TODO: VM release hook
            if (!sq_isnull(obj_))
            {
                sq_release(vm_, &obj_);
                sq_resetobject(&obj_);
            }
        }
    };
}

#endif
