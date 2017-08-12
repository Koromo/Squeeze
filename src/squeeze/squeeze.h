#ifndef SQUEEZE_H
#define SQUEEZE_H

#define _CRT_SECURE_NO_WARNINGS

/**
The case of defined this macro, then Squeeze use double type for the real number type.
Otherwise (default), Squeeze use float type.
*/
#ifdef SQZ_DOUBLE
#define SQUSEDOUBLE
#endif

#include "sqzscript.h"
#include "sqzclass.h"
#include "sqztable.h"
#include "sqzobject.h"
#include "sqzvm.h"
#include "sqzstackop.h"
#include "sqzdef.h"
#include "sqzutil.h"

#include "sqzimpl.h"

#endif

