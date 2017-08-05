#ifndef SQUEEZE_SQZDEF_H
#define SQUEEZE_SQZDEF_H

#include "../squirrel/squirrel.h"
#include <tuple>
#include <exception>
#include <stdexcept>

#define SQZ_T(s) _SC(s)

namespace squeeze
{
    /// Character set
    using char_t = SQChar;

    /// Type tags
    enum class TypeTag
    {
        Null = OT_NULL,
        Integer = OT_INTEGER,
        Float = OT_FLOAT,
        Bool = OT_BOOL,
        String = OT_STRING,
        Table = OT_TABLE,
        Closure = OT_NATIVECLOSURE
    };
}

#endif
