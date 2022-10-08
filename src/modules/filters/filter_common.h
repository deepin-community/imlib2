#ifndef __FILTER_COMMON_H
#define __FILTER_COMMON_H 1

#include "config.h"
#include "common.h"
#include "dynamic_filters.h"
#include "image.h"

__EXPORT__ void     init(struct imlib_filter_info *info);
__EXPORT__ void     deinit(void);
__EXPORT__ void    *exec(char *filter, void *im, pIFunctionParam params);

#endif /* __FILTER_COMMON_H */
