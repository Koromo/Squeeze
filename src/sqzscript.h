#ifndef SQUEEZE_SQZSQRIPT_H
#define SQUEEZE_SQZSQRIPT_H

#include "sqztable.h"
#include "sqzobject.h"
#include "sqzdef.h"
#include "sqzutil.h"
#include <squirrel.h>
#include <sqstdio.h>

namespace squeeze
{
    /** The script objetc handle */
    class HScript : public HObject
    {
    public:
        /** Construct */
        HScript() = default;

        /** Construct */
        explicit HScript(HVM vm)
        {
            vm_ = vm;
        }

        /** Compile script code */
        void compileFile(const string_t& path)
        {
            release();
            if (SQ_FAILED(sqstd_loadfile(vm_, path.c_str(), SQTrue)))
            {
                failed<ScriptException>(vm_, "sqstd_loadfile() failed.");
            }
            sq_getstackobj(vm_, -1, &obj_);
            sq_addref(vm_, &obj_);
            sq_poptop(vm_);
        }

        /** Run the compiled script */
        void run(HTable env)
        {
            if (!sq_isnull(obj_))
            {
                pushValue(vm_, obj_, env);
                if (SQ_FAILED(sq_call(vm_, 1, SQFalse, SQTrue)))
                {
                    sq_poptop(vm_);
                    failed<ScriptException>(vm_, "sq_call() failed.");
                }
                sq_poptop(vm_);
            }
        }
    };
}

#endif