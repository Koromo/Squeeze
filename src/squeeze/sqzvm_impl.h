#ifndef SQUEEZE_SQZVM_IMPL_H
#define SQUEEZE_SQZVM_IMPL_H

#include "sqztable.h"
#include "sqzdef.h"
#include "../squirrel/squirrel.h"

namespace squeeze
{
    HTable HVM::rootTable()
    {
        HSQOBJECT root;
        sq_pushroottable(vm_);
        sq_getstackobj(vm_, -1, &root);
        sq_addref(vm_, &root);
        sq_poptop(vm_);
        return HTable(*this, root);
    }
    
    void HVM::setRootTable(HTable root)
    {
        sq_pushobject(vm_, root);
        sq_setroottable(vm_);
    }
}

#endif

