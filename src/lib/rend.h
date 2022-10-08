#ifndef __REND
#define __REND 1

#include "common.h"

DATA32              __imlib_RenderGetPixel(Display * d, Drawable w, Visual * v,
                                           Colormap cm, int depth, DATA8 r,
                                           DATA8 g, DATA8 b);

void                __imlib_RenderDisconnect(Display * d);

void                __imlib_RenderImage(Display * d, ImlibImage * im,
                                        Drawable w, Drawable m,
                                        Visual * v, Colormap cm, int depth,
                                        int sx, int sy, int sw, int sh,
                                        int dx, int dy, int dw, int dh,
                                        char anitalias, char hiq, char blend,
                                        char dither_mask, int mat,
                                        ImlibColorModifier * cmod, ImlibOp op);

void                __imlib_RenderImageSkewed(Display * d, ImlibImage * im,
                                              Drawable w, Drawable m,
                                              Visual * v, Colormap cm,
                                              int depth, int sx, int sy, int sw,
                                              int sh, int dx, int dy, int hsx,
                                              int hsy, int vsx, int vsy,
                                              char antialias, char hiq,
                                              char blend, char dither_mask,
                                              int mat,
                                              ImlibColorModifier * cmod,
                                              ImlibOp op);

#endif
