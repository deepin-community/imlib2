AUTOMAKE_OPTIONS     = 1.4 foreign
MAINTAINERCLEANFILES = Makefile.in
AM_CFLAGS            = $(CFLAGS_WARNINGS) $(CFLAGS_VISIBILITY) $(CFLAGS_ASAN)
AM_CPPFLAGS          = -DPACKAGE_LIB_DIR=\"$(libdir)\" \
                       -I$(top_builddir) \
                       $(X_CFLAGS) \
                       $(FREETYPE_CFLAGS)

# Older automakes (<1.10) do not use INCLUDES when compiling .S files
AM_CCASFLAGS = -I$(top_builddir)

lib_LTLIBRARIES      = libImlib2.la
include_HEADERS      = Imlib2.h
libImlib2_la_SOURCES = \
api.c \
asm.h \
blend.c \
blend.h \
color.c \
color.h \
color_helpers.c \
color_helpers.h \
colormod.c \
colormod.h \
common.h \
context.c \
context.h \
draw.c \
draw.h \
dynamic_filters.c \
dynamic_filters.h \
ellipse.c \
file.c \
file.h \
filter.c \
filter.h \
font.h \
font_draw.c \
font_load.c \
font_main.c \
font_query.c \
grab.c \
grab.h \
grad.c \
grad.h \
image.c \
image.h \
image_tags.c \
line.c \
loaders.c \
loaders.h \
modules.c \
polygon.c \
rectangle.c \
rend.c \
rend.h \
rgba.c \
rgba.h \
rgbadraw.c \
rgbadraw.h \
rotate.c \
rotate.h \
scale.c \
scale.h \
script.c \
script.h \
span.c \
span.h \
updates.c \
updates.h \
ximage.c \
ximage.h

MMX_SRCS = \
asm_blend.S \
asm_blend_cmod.S \
asm_rgba.S \
asm_rotate.S \
asm_scale.S

AMD64_SRCS = \
amd64_blend.S \
amd64_blend_cmod.S

MMX_OBJS = $(MMX_SRCS:.S=.lo)
AMD64_OBJS = $(AMD64_SRCS:.S=.lo)

EXTRA_DIST = $(MMX_SRCS) $(AMD64_SRCS) asm_loadimmq.S

MY_LIBS = $(FREETYPE_LIBS) $(DLOPEN_LIBS) -lm
if BUILD_X11
MY_LIBS += -lXext -lX11 @X_SHM_FD_LIBS@
endif

if BUILD_MMX
libImlib2_la_LIBADD       = $(MMX_OBJS) $(MY_LIBS)
libImlib2_la_DEPENDENCIES = $(top_builddir)/config.h $(MMX_OBJS)
EXTRA_libImlib2_la_SOURCES = $(MMX_SRCS) 
else
if BUILD_AMD64
libImlib2_la_LIBADD       = $(AMD64_OBJS) $(MY_LIBS)
libImlib2_la_DEPENDENCIES = $(top_builddir)/config.h $(AMD64_OBJS)
EXTRA_libImlib2_la_SOURCES = $(AMD64_SRCS)
else
libImlib2_la_LIBADD       = $(MY_LIBS)
libImlib2_la_DEPENDENCIES = $(top_builddir)/config.h
endif
endif
libImlib2_la_LDFLAGS      = -version-info @lt_version@

