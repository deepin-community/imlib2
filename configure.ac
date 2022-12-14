AC_INIT([imlib2],[1.7.4],[enlightenment-devel@lists.sourceforge.net])
AC_CONFIG_SRCDIR(configure.ac)
AC_CONFIG_MACRO_DIR([m4])
AC_CANONICAL_BUILD
AC_CANONICAL_HOST

AM_INIT_AUTOMAKE(1.6 dist-bzip2)
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AM_CONDITIONAL(SRC_IS_GIT, test -d $srcdir/.git)

AC_C_BIGENDIAN
AC_PROG_CC
AM_PROG_AS

AC_C_CONST

dnl Next two lines is a hack to prevent libtool checking for CXX/F77
define([AC_LIBTOOL_LANG_CXX_CONFIG], [:])dnl
define([AC_LIBTOOL_LANG_F77_CONFIG], [:])dnl
LT_INIT

VERSION_CUR=8
VERSION_REV=4
VERSION_AGE=7
lt_version=${VERSION_CUR}:${VERSION_REV}:${VERSION_AGE}
AC_SUBST(lt_version)

PKG_PROG_PKG_CONFIG


mmx=no
amd64=no
case $host_cpu in
  i*86)   mmx="yes";;
  x86_64) amd64="yes";;
  amd64) amd64="yes";;
esac

case $host_os in
  solaris*) mmx="no";;
  darwin*) mmx="no";;
esac

AC_ARG_ENABLE([mmx],
  [AS_HELP_STRING([--enable-mmx], [attempt compiling using x86 mmx assembly @<:@default=auto@:>@])],
  [
   if test x$enableval = xyes; then
     mmx=yes
     # Cannot compile with both options enabled
     amd64=no
   else
     mmx=no
   fi
  ]
)

AC_ARG_ENABLE([amd64],
  [AS_HELP_STRING([--enable-amd64], [attempt compiling using amd64 assembly @<:@default=auto@:>@])],
  [
   if test x$enableval = xyes; then
     amd64=yes
     mmx=no
   else
     amd64=no
   fi
  ]
)

AC_MSG_CHECKING(whether to enable x86 mmx support)
if test x$mmx = xyes; then
  AC_DEFINE(DO_MMX_ASM, 1, [enabling MMX Assembly])
fi
AC_MSG_RESULT($mmx)
AM_CONDITIONAL(BUILD_MMX, test x$mmx = xyes)

AC_MSG_CHECKING(whether to enable amd64 asm support)
if test x$amd64 = xyes; then
  AC_DEFINE(DO_AMD64_ASM, 1, [enabling AMD64 Assembly])
fi
AC_MSG_RESULT($amd64)
AM_CONDITIONAL(BUILD_AMD64, test x$amd64 = xyes)

# check for freetype
PKG_CHECK_MODULES(FREETYPE, freetype2)

AC_PATH_X
AC_PATH_XTRA

if test "x$have_x" = "xno"; then
   # If --without-x was passed, this will be "disabled" instead of "no" --
   # so always treat "no" as an error
   AC_MSG_ERROR(
[no X support found.  Use --x-includes and --x-libraries to specify the X
path on your system, or --without-x to disable X support.])
fi

if test "x$have_x" = "xyes"; then
  AC_DEFINE(BUILD_X11, 1, [enabling X11 support])
  AC_MSG_CHECKING(whether to enable X11 MIT-SHM FD-passing support)
  AC_ARG_WITH([x-shm-fd],
    [AS_HELP_STRING([--without-x-shm-fd], [Disable X11 MIT-SHM FD-passing support])],
    [
     if test "$withval" = no ; then
       x_shm_fd=no
     else
       x_shm_fd=yes
     fi
    ],
    [ x_shm_fd=auto ]
  )
  AC_MSG_RESULT($x_shm_fd)

  if test "$x_shm_fd" != no ; then
    PKG_CHECK_MODULES(X_SHM_FD, x11-xcb xcb xcb-shm >= 1.9, [ x_shm_fd="yes" ], [ x_shm_fd="no"])
  else
    x_shm_fd="no"
  fi
  if test "$x_shm_fd" = yes ; then
    AC_DEFINE(HAVE_X11_SHM_FD, 1, [enabling X11 MIT-SHM FD-passing support])
  fi
else
  have_x="no"
  x_shm_fd="no"
fi
AM_CONDITIONAL(BUILD_X11, test "x$have_x" = "xyes")

AC_CHECK_LIB(dl, dlopen, DLOPEN_LIBS=-ldl)
AC_SUBST(DLOPEN_LIBS)


AC_MSG_CHECKING(whether to enable jpeg support)
AC_ARG_WITH([jpeg],
  [AS_HELP_STRING([--without-jpeg], [Disable JPEG image loader])],
  [
   if test "$withval" = no ; then
     jpeg_loader=no
   else
     jpeg_loader=yes
   fi
  ],
  [ jpeg_loader=auto ]
)
AC_MSG_RESULT($jpeg_loader)

if test "$jpeg_loader" != no ; then
  PKG_CHECK_MODULES(JPEG, libjpeg, [ jpeg_ok="yes" ], [ jpeg_ok="no" ])
  if test "$jpeg_ok" = no ; then
    if test "$jpeg_loader" = yes ; then
      AC_MSG_ERROR(JPEG support was requested but system does not support it)
    else
      AC_MSG_WARN(*** Native JPEG support will not be built (JPEG not found) ***)
    fi
  fi
else
  jpeg_ok=no
fi
AM_CONDITIONAL(BUILD_JPEG_LOADER, test "$jpeg_ok" = yes)


AC_MSG_CHECKING(whether to enable png support)
AC_ARG_WITH([png],
  [AS_HELP_STRING([--without-png], [Disable PNG image loader])],
  [
   if test "$withval" = no ; then
     png_loader=no
   else
     png_loader=yes
   fi
  ],
  [ png_loader=auto ]
)
AC_MSG_RESULT($png_loader)

if test "$png_loader" != no ; then
  PKG_CHECK_MODULES(PNG, libpng, [ png_ok="yes" ], [
    PKG_CHECK_MODULES(PNG, libpng12, [ png_ok="yes" ], [
      PKG_CHECK_MODULES(PNG, libpng10, [ png_ok="yes" ], [ png_ok="no"])
    ])
  ])
else
  png_ok="no"
fi
AM_CONDITIONAL(BUILD_PNG_LOADER, test "$png_ok" = yes)

AC_MSG_CHECKING(whether to enable webp support)
AC_ARG_WITH([webp],
  [AS_HELP_STRING([--without-webp], [Disable WEBP image loader])],
  [
   if test "$withval" = no ; then
     webp_loader=no
   else
     webp_loader=yes
   fi
  ],
  [ webp_loader=auto ]
)
AC_MSG_RESULT($webp_loader)

if test "$webp_loader" != no ; then
  PKG_CHECK_MODULES(WEBP, libwebp, [ webp_ok="yes" ], [ webp_ok="no" ])
else
  webp_ok="no"
fi
AM_CONDITIONAL(BUILD_WEBP_LOADER, test "$webp_ok" = yes)


AC_MSG_CHECKING(whether to enable tiff support)
AC_ARG_WITH([tiff],
  [AS_HELP_STRING([--without-tiff], [Disable TIFF image loader])],
  [
   if test "$withval" = no ; then
     tiff_loader=no
   else
     tiff_loader=yes
   fi
  ],
  [ tiff_loader=auto ]
)
AC_MSG_RESULT($tiff_loader)

if test "$tiff_loader" != no ; then
  PKG_CHECK_MODULES(TIFF, libtiff-4, [ tiff_ok="yes" ], [ tiff_ok="no" ])
  if test "$tiff_ok" = no ; then
    if test "$tiff_loader" = yes ; then
      AC_MSG_ERROR(TIFF support was requested but system does not support it)
    else
      AC_MSG_WARN(*** Native TIFF support will not be built (TIFF not found) ***)
    fi
  fi
else
  tiff_ok=no
fi
AM_CONDITIONAL(BUILD_TIFF_LOADER, test "$tiff_ok" = yes)


AC_MSG_CHECKING(whether to enable gif support)
gif_lib=any
AC_ARG_WITH([gif],
  [AS_HELP_STRING([--without-gif], [Disable GIF image loader])],
  [
   if test "$withval" = no ; then
     gif_loader=no
   else
     gif_loader=yes
     case "$withval" in
       giflib)   gif_lib=giflib;;
       libungif) gif_lib=libungif;;
     esac
   fi
  ],
  [ gif_loader=auto ]
)
AC_MSG_RESULT($gif_loader)

if test "$gif_loader" != no ; then
  AC_MSG_CHECKING(for preferred gif provider)
  AC_MSG_RESULT($gif_lib)

  if test "$gif_lib" != libungif ; then
    AC_CHECK_LIB(gif, DGifOpenFileName, gif_libs="-lgif" gif_ok=yes, gif_ok=no)
    if test "$gif_ok" = yes; then
      AC_CHECK_HEADER([gif_lib.h], , gif_ok=no)
      if test "$gif_ok" = yes; then
        GIFLIBS=$gif_libs
        gif_lib=giflib
      fi
    fi
  fi

  if test "$gif_lib" != giflib ; then
    AC_CHECK_LIB(ungif, DGifOpenFileName, gif_libs="-lungif" gif_ok=yes, gif_ok=no)
    if test "$gif_ok" = yes; then
      AC_CHECK_HEADER([gif_lib.h], , gif_ok=no)
      if test "$gif_ok" = yes; then
        GIFLIBS=$gif_libs
        gif_lib=libungif
      fi
    fi
  fi

  if test "$gif_ok" = no ; then
    if test "$gif_loader" = yes ; then
      AC_MSG_ERROR(GIF support was requested but system does not support it)
    else
      AC_MSG_WARN(*** Native GIF support will not be built (GIF not found) ***)
    fi
  fi
else
  gif_ok=no
fi
AM_CONDITIONAL(BUILD_GIF_LOADER, test "$gif_ok" = yes)
AC_SUBST(GIFLIBS)


AC_MSG_CHECKING(whether to enable zlib support)
AC_ARG_WITH([zlib],
  [AS_HELP_STRING([--without-zlib], [Disable ZLIB loader])],
  [
   if test "$withval" = no ; then
     zlib_loader=no
   else
     zlib_loader=yes
   fi
  ],
  [ zlib_loader=auto ]
)
AC_MSG_RESULT($zlib_loader)

if test "$zlib_loader" != no ; then
  PKG_CHECK_MODULES(ZLIB, zlib, [ zlib_ok="yes" ], [ zlib_ok="no" ])
  if test "$zlib_ok" = no ; then
    if test "$zlib_loader" = yes ; then
      AC_MSG_ERROR(ZLIB support was requested but system does not support it)
    else
      AC_MSG_WARN(*** Native zlib support will not be built (zlib not found) ***)
    fi
  fi
else
  zlib_ok=no
fi
AM_CONDITIONAL(BUILD_ZLIB_LOADER, test "$zlib_ok" = yes)


AC_MSG_CHECKING(whether to enable bzip2 support)
AC_ARG_WITH([bzip2],
  [AS_HELP_STRING([--without-bzip2], [Disable BZIP2 loader])],
  [
   if test "$withval" = no ; then
     bz2_loader=no
   else
     bz2_loader=yes
   fi
  ],
  [ bz2_loader=auto ]
)
AC_MSG_RESULT($bz2_loader)

if test "$bz2_loader" != no ; then
  PKG_CHECK_MODULES(BZ2, bzip2, [ bz2_ok="yes" ], [ bz2_ok="no" ])
  if test "$bz2_ok" = no ; then
    AC_CHECK_LIB(bz2, BZ2_bzRead, bz2_ok=yes, bz2_ok=no)
    if test "$bz2_ok" = yes; then
      AC_CHECK_HEADER([bzlib.h], , bz2_ok=no)
      if test "$bz2_ok" = yes; then
        BZ2_LIBS="-lbz2"
      fi
    fi
  fi
  if test "$bz2_ok" = no ; then
    if test "$bz2_loader" = yes ; then
      AC_MSG_ERROR(BZIP2 support was requested but system does not support it)
    else
      AC_MSG_WARN(*** Native bz2 support will not be built (bzip2 not found) ***)
    fi
  fi
else
  bz2_ok=no
fi
AM_CONDITIONAL(BUILD_BZ2_LOADER, test "$bz2_ok" = yes)


AC_MSG_CHECKING(whether to enable id3 support)
AC_ARG_WITH([id3],
  [AS_HELP_STRING([--without-id3], [Disable ID3 loader])],
  [
   if test "$withval" = no ; then
     id3_loader=no
   else
     id3_loader=yes
   fi
  ],
  [ id3_loader=auto ]
)
AC_MSG_RESULT($id3_loader)

if test "$id3_loader" != no ; then
  PKG_CHECK_MODULES(ID3, id3tag, [ id3_ok="yes" ], [ id3_ok="no" ])
  if test "$id3_ok" = no ; then
    if test "$id3_loader" = yes ; then
      AC_MSG_ERROR(ID3 support was requested but system does not support it)
    else
      AC_MSG_WARN(*** Native mp3 support will not be built (id3tag not found) ***)
    fi
  fi
else
  id3_ok=no
fi
AM_CONDITIONAL(BUILD_ID3_LOADER, test "$id3_ok" = yes)


EC_C_WARNINGS()
EC_C_VISIBILITY(yes)
EC_C_ASAN()

AC_CONFIG_HEADERS(config.h)
AC_CONFIG_FILES([
Makefile
imlib2.pc
src/Makefile
src/lib/Makefile
src/bin/Makefile
src/modules/Makefile
src/modules/filters/Makefile
src/modules/loaders/Makefile
data/Makefile
data/fonts/Makefile
data/images/Makefile
doc/Makefile
imlib2-config
README
])
AC_CONFIG_COMMANDS([default],[
chmod +x imlib2-config
],[])
AC_OUTPUT

#####################################################################
## Info
echo
echo
echo "------------------------------------------------------------------------"
echo "$PACKAGE $VERSION"
echo "------------------------------------------------------------------------"
echo
echo "Configuration Options Summary:"
echo
echo "Image loaders:"
echo "  JPEG....................: $jpeg_ok"
echo "  PNG.....................: $png_ok"
echo "  TIFF....................: $tiff_ok"
echo "  GIF.....................: $gif_ok"
echo "  WEBP....................: $webp_ok"
echo "  ZLIB....................: $zlib_ok"
echo "  BZIP2...................: $bz2_ok"
echo "  ID3.....................: $id3_ok"
echo
echo "Build for X11.............: $have_x"
echo "Use X MIT-SHM FD-passing..: $x_shm_fd"
echo
echo "Use X86 MMX for speed.....: $mmx"
echo "Use AMD64 for speed.......: $amd64"
echo
echo "Use visibility hiding.....: $enable_visibility_hiding"
echo
echo "Installation path.........: $prefix"
echo
