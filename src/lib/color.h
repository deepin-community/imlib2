#ifndef __COLOR
#define __COLOR 1

#include "common.h"

#ifdef BUILD_X11

extern DATA16       _max_colors;

int                 __imlib_XActualDepth(Display * d, Visual * v);
Visual             *__imlib_BestVisual(Display * d, int screen,
                                       int *depth_return);
DATA8              *__imlib_AllocColorTable(Display * d, Colormap cmap,
                                            DATA8 * type_return, Visual * v);
DATA8              *__imlib_AllocColors332(Display * d, Colormap cmap,
                                           Visual * v);
DATA8              *__imlib_AllocColors666(Display * d, Colormap cmap,
                                           Visual * v);
DATA8              *__imlib_AllocColors232(Display * d, Colormap cmap,
                                           Visual * v);
DATA8              *__imlib_AllocColors222(Display * d, Colormap cmap,
                                           Visual * v);
DATA8              *__imlib_AllocColors221(Display * d, Colormap cmap,
                                           Visual * v);
DATA8              *__imlib_AllocColors121(Display * d, Colormap cmap,
                                           Visual * v);
DATA8              *__imlib_AllocColors111(Display * d, Colormap cmap,
                                           Visual * v);
DATA8              *__imlib_AllocColors1(Display * d, Colormap cmap,
                                         Visual * v);

#endif

#endif
