dnl ***
dnl *** Process this file with autoconf to produce a configure script.
dnl ***

dnl Initialize
dnl ==========
AC_PREREQ([2.59])
AC_INIT([audacious-plugins], [3.4-devel])
AC_COPYRIGHT([(C) 2005-2012 Audacious Team])

AC_SUBST([PACKAGE], [AC_PACKAGE_TARNAME])
AC_SUBST([VERSION], [AC_PACKAGE_VERSION])
AC_DEFINE_UNQUOTED([PACKAGE], "$PACKAGE", [Name of package])
AC_DEFINE_UNQUOTED([VERSION], "$VERSION", [Version number of package])

AC_CANONICAL_HOST
AC_CANONICAL_TARGET
AC_CONFIG_HEADERS([config.h])

AUD_COMMON_PROGS

BUILDSYS_INIT
BUILDSYS_SHARED_LIB

dnl Headers and functions
dnl =====================
AC_CHECK_FUNCS([fcntl fsync mkdtemp])

dnl gettext
dnl =======
AM_GNU_GETTEXT([external])
if test "$MSGFMT" = ":" ; then
    AC_MSG_ERROR([msgfmt was not found; have you installed gettext?])
fi

LIBS="$LIBS $LIBINTL"

dnl Check for Audacious
dnl ===================

PKG_CHECK_MODULES(AUDACIOUS, [audacious >= 3.4],
    [],
    [AC_MSG_ERROR([Cannot find Audacious 3.4; have you installed Audacious yet?])]
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
 [  --disable-usf           disable the USF input plugin],
 [enable_usf=$enableval],
 [enable_usf=yes])

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
AC_SUBST(pluginsubs)

dnl XXX Work around some autoconf bugs.
if test "x$prefix" = "xNONE"; then
        prefix="${ac_default_prefix}"
fi

if test -z "$datadir"; then
	datadir="${prefix}/share"
fi

localedir="$datadir/locale"
AC_SUBST(localedir)

AC_SUBST(EFFECT_PLUGINS)
AC_SUBST(GENERAL_PLUGINS)
AC_SUBST(INPUT_PLUGINS)
AC_SUBST(OUTPUT_PLUGINS)
AC_SUBST(VISUALIZATION_PLUGINS)
AC_SUBST(CONTAINER_PLUGINS)
AC_SUBST(TRANSPORT_PLUGINS)
AC_SUBST(GCC42_CFLAGS)

AC_CONFIG_FILES([
	buildsys.mk
	extra.mk
])

BUILDSYS_TOUCH_DEPS
AC_OUTPUT

