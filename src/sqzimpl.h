#ifndef SQUEEZE_SQZIMPL_H
#define SQUEEZE_SQZIMPL_H

#include "sqztable.h"
#include "sqzclass.h"
#include "sqzdef.h"
#include <squirrel.h>

namespace squeeze
{
    inline HTable HVM::rootTable()
    {
        HSQOBJECT root;
        sq_pushroottable(vm_);
        sq_getstackobj(vm_, -1, &root);
        sq_poptop(vm_);
        return HTable(*this, root);
    }
    
    inline void HVM::setRootTable(HTable root)
    {
        sq_pushobject(vm_, root);
        sq_setroottable(vm_);
    }

    template <class Class> HTable& HTable::clazz(const string_t& key, HClass<Class> c)
    {
        newSlot(key, c, false);
        return *this;
    }
}

#endif

