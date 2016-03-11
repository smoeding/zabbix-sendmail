#                              -*- Autoconf -*-

AC_PREREQ([2.69])
AC_INIT([zabbix-sendmail], [0.1.0], [stm@kill-9.net])

# Prepare automake
AM_INIT_AUTOMAKE([1.11 foreign])

LT_INIT

AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile])


# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL

# Checks for libraries.

# Checks for header files.
AC_HEADER_RESOLV
AC_CHECK_HEADER(errno.h)
AC_CHECK_HEADER(fcntl.h)
AC_CHECK_HEADER(stdint.h)
AC_CHECK_HEADER(stdlib.h)
AC_CHECK_HEADER(string.h)
AC_CHECK_HEADER(sys/stat.h)
AC_CHECK_HEADER(sys/types.h)
AC_CHECK_HEADER(unistd.h)

# Checking for Zabbix headers
AX_LIB_ZABBIX
if test ! "x$found_zabbix" = "xyes"; then
   AC_MSG_ERROR([Zabbix headers not found])
fi

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT32_T

# Checks for library functions.


AC_OUTPUT
