AC_INIT([smudge], [0.3.0], [smudge.language@gmail.com])
AC_PREFIX_DEFAULT(/usr)
AC_CONFIG_AUX_DIR([scripts])
AM_INIT_AUTOMAKE([subdir-objects])
: ${CXXFLAGS=""}
AC_PROG_CXX
AC_CANONICAL_HOST
AC_CONFIG_FILES([Makefile])

case "${host_os}" in
    cygwin*|mingw*)
        is_windows=true
        ;;
esac

AM_CONDITIONAL([IS_WINDOWS], [test "$is_windows" = "true"])
AC_OUTPUT
