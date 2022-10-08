#ifndef __SPAN
#define __SPAN 1

#include "common.h"

typedef void        (*ImlibPointDrawFunction)(DATA32, DATA32 *);

ImlibPointDrawFunction
__imlib_GetPointDrawFunction(ImlibOp op, char dst_alpha, char blend);

typedef void        (*ImlibSpanDrawFunction)(DATA32, DATA32 *, int);

ImlibSpanDrawFunction
__imlib_GetSpanDrawFunction(ImlibOp op, char dst_alpha, char blend);

typedef void        (*ImlibShapedSpanDrawFunction)(DATA8 *, DATA32, DATA32 *,
                                                   int);

ImlibShapedSpanDrawFunction
__imlib_GetShapedSpanDrawFunction(ImlibOp op, char dst_alpha, char blend);

#endif
