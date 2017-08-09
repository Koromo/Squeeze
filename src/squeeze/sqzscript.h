#ifndef SQUEEZE_SQZSQRIPT_H
#define SQUEEZE_SQZSQRIPT_H

#include "sqztable.h"
#include "sqzobject.h"
#include "sqzdef.h"
#include "squirrel/squirrel.h"
#include "squirrel/sqstdio.h"

namespace squeeze
{
    class ScriptException : std::runtime_error
    {
    public:
        explicit ScriptException(const std::string& msg = "script exception")
            : std::runtime_error(msg) {}
    };

    class HScript : public HObject
    {
    public:
        /// Construct
        HScript() = default;

        /// ditto
        explicit HScript(HVM vm)
        {
            vm_ = vm;
        }

        /// Compile
        void compileFile(string_t path)
        {
            release();
            if (SQ_FAILED(sqstd_loadfile(vm_, sq(path), SQTrue)))
            {
                throw ScriptException();
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
                pushValue(vm_, obj_, env);
                if (SQ_FAILED(sq_call(vm_, 1, SQFalse, SQTrue)))
                {
                    sq_pop(vm_, 2);
                    throw ScriptException();
                }
                sq_pop(vm_, 2);
            }
        }
    };
}

#endif