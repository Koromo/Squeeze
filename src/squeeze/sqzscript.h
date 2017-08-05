#ifndef SQUEEZE_SQZSQRIPT_H
#define SQUEEZE_SQZSQRIPT_H

#include "sqztable.h"
#include "sqzobject.h"
#include "sqzdef.h"
#include "../squirrel/squirrel.h"
#include "../squirrel/sqstdio.h"

namespace squeeze
{
    class HScript : public HObject
    {
    public:
        /// Create a script
        explicit HScript(HVM vm)
            : HObject(vm)
        {
        }

        /// Compile
        void compileFile(const char_t* path)
        {
            release();
            if (SQ_FAILED(sqstd_loadfile(vm_, path, SQTrue)))
            {
                throw std::runtime_error("Failed to compile.");
            }
            sq_getstackobj(vm_, -1, &obj_);
            sq_addref(vm_, &obj_);
            sq_poptop(vm_);
        }

        /// Run
        void run(HTable env)
        {
            if (!sq_isnull(obj_))
            {
                sq_pushobject(vm_, obj_);
                sq_pushobject(vm_, env);
                if (SQ_FAILED(sq_call(vm_, 1, false, true)))
                {
                    sq_pop(vm_, 2);
                    throw std::runtime_error("Failed to run script.");
                }
                sq_pop(vm_, 2);
            }
        }
    };
}

#endif