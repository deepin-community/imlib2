#ifndef __LOADER_COMMON_H
#define __LOADER_COMMON_H 1

#include "config.h"
#include "common.h"
#include "image.h"

__EXPORT__ char     load(ImlibImage * im, ImlibProgressFunction progress,
                         char progress_granularity, char load_data);
__EXPORT__ int      load2(ImlibImage * im, int load_data);
__EXPORT__ char     save(ImlibImage * im, ImlibProgressFunction progress,
                         char progress_granularity);
__EXPORT__ void     formats(ImlibLoader * l);

#endif /* __LOADER_COMMON_H */
