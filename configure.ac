dnl ***
dnl *** Process this file with autoconf to produce a configure script.
dnl ***

dnl Initialize
dnl ==========
AC_PREREQ([2.59])
AC_INIT([audacious-plugins], [3.5])
AC_COPYRIGHT([Copyright (C) 2001-2015 Audacious developers and others])

AC_DEFINE_UNQUOTED([PACKAGE], "$PACKAGE_NAME", [Name of package])
AC_DEFINE_UNQUOTED([VERSION], "$PACKAGE_VERSION", [Version number of package])

AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AUD_COMMON_PROGS

BUILDSYS_SHARED_LIB

dnl Headers and functions
dnl =====================
AC_CHECK_FUNCS([mkdtemp])

dnl gettext
dnl =======
AM_GNU_GETTEXT([external])
if test "$MSGFMT" = ":" ; then
    AC_MSG_ERROR([msgfmt was not found; have you installed gettext?])
fi

LIBS="$LIBS $LIBINTL"

dnl Check for Audacious
dnl ===================

PKG_CHECK_MODULES(AUDACIOUS, [audacious >= 3.5],
    [],
    [AC_MSG_ERROR([Cannot find Audacious 3.5; have you installed Audacious yet?])]
)

CPPFLAGS="$CPPFLAGS $AUDACIOUS_CFLAGS"
LIBS="$LIBS $AUDACIOUS_LIBS"

dnl Default Set of Plugins
dnl ======================

INPUT_PLUGINS=""
OUTPUT_PLUGINS=""
EFFECT_PLUGINS=""
GENERAL_PLUGINS=""
VISUALIZATION_PLUGINS=""
CONTAINER_PLUGINS=""
TRANSPORT_PLUGINS=""

dnl USF
dnl ===

AC_ARG_ENABLE(usf,
 [AS_HELP_STRING([--disable-usf], [disable the USF input plugin])],
 [enable_usf=$enableval], [enable_usf=yes])

if test "$enable_usf" = "yes"; then
    AC_MSG_CHECKING([if your platform is supported for the USF plugin])
    case "$target" in
	x86_64-*-linux-gnu)
		AC_MSG_RESULT([yes, AMD64 Linux])
		enable_usf=yes
		;;
	i*86-*-linux-gnu)
		AC_MSG_RESULT([yes, X86 Linux])
		enable_usf=yes
		;;
	*)
		AC_MSG_RESULT([no])
		enable_usf=no
		;;
    esac

    if test "$enable_usf" = "yes"; then
        INPUT_PLUGINS="$INPUT_PLUGINS usf"
    fi
fi

dnl *** End of all plugin checks ***

plugindir=`pkg-config audacious --variable=plugin_dir`
AC_SUBST(plugindir)

dnl XXX

INPUT_PLUGIN_DIR=Input
OUTPUT_PLUGIN_DIR=Output
EFFECT_PLUGIN_DIR=Effect
GENERAL_PLUGIN_DIR=General
VISUALIZATION_PLUGIN_DIR=Visualization
CONTAINER_PLUGIN_DIR=Container
TRANSPORT_PLUGIN_DIR=Transport

AC_SUBST(INPUT_PLUGIN_DIR)
AC_SUBST(OUTPUT_PLUGIN_DIR)
AC_SUBST(EFFECT_PLUGIN_DIR)
AC_SUBST(GENERAL_PLUGIN_DIR)
AC_SUBST(VISUALIZATION_PLUGIN_DIR)
AC_SUBST(CONTAINER_PLUGIN_DIR)
AC_SUBST(TRANSPORT_PLUGIN_DIR)

localedir="$datarootdir/locale"
AC_SUBST(localedir)

AC_SUBST(EFFECT_PLUGINS)
AC_SUBST(GENERAL_PLUGINS)
AC_SUBST(INPUT_PLUGINS)
AC_SUBST(OUTPUT_PLUGINS)
AC_SUBST(VISUALIZATION_PLUGINS)
AC_SUBST(CONTAINER_PLUGINS)
AC_SUBST(TRANSPORT_PLUGINS)


dnl Reliably #include "config.h" (for large file support)
dnl =====================================================
CPPFLAGS="$CPPFLAGS -include config.h"


dnl Generate config files
dnl =====================

AC_CONFIG_FILES([
    buildsys.mk
    extra.mk
])

AC_OUTPUT


dnl Print results
dnl =============

echo
echo "Configuration:"
echo
echo "  Install path:                           $plugindir"
echo
echo "  Input Plugins"
echo "  -------------"
echo "  Nintendo 64 music (usf, miniusf):       $enable_usf"
echo
