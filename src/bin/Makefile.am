MAINTAINERCLEANFILES = Makefile.in

AM_CFLAGS   = $(CFLAGS_WARNINGS) $(CFLAGS_ASAN)
AM_CPPFLAGS = \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
-I$(top_srcdir)/src/lib \
$(X_CFLAGS)

if BUILD_X11
X_BASED_PROGS = imlib2_show imlib2_test imlib2_bumpmap imlib2_poly imlib2_colorspace imlib2_view imlib2_grab
endif

bin_PROGRAMS = \
imlib2_conv \
imlib2_load \
$(X_BASED_PROGS)

imlib2_conv_SOURCES = imlib2_conv.c
imlib2_conv_LDADD   = $(top_builddir)/src/lib/libImlib2.la

imlib2_load_SOURCES = imlib2_load.c
imlib2_load_LDADD   = $(top_builddir)/src/lib/libImlib2.la

imlib2_show_SOURCES = imlib2_show.c
imlib2_show_LDADD   = $(top_builddir)/src/lib/libImlib2.la -lX11 -lm

imlib2_test_SOURCES = imlib2_test.c
imlib2_test_LDADD   = $(top_builddir)/src/lib/libImlib2.la -lX11

imlib2_bumpmap_SOURCES = imlib2_bumpmap.c
imlib2_bumpmap_LDADD   = $(top_builddir)/src/lib/libImlib2.la -lX11

imlib2_poly_SOURCES = imlib2_poly.c
imlib2_poly_LDADD   = $(top_builddir)/src/lib/libImlib2.la -lX11

imlib2_colorspace_SOURCES = imlib2_colorspace.c
imlib2_colorspace_LDADD   = $(top_builddir)/src/lib/libImlib2.la -lX11

imlib2_view_SOURCES = imlib2_view.c props.c props.h
imlib2_view_LDADD   = $(top_builddir)/src/lib/libImlib2.la -lX11

imlib2_grab_SOURCES = imlib2_grab.c
imlib2_grab_LDADD   = $(top_builddir)/src/lib/libImlib2.la -lX11
