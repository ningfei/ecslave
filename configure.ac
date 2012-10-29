AC_PREREQ([2.65])
AC_INIT([ec_slave], [0.1], [raziebe@gmail.com])
AC_CONFIG_SRCDIR([src/])
AC_CONFIG_FILES([Makefile])
AM_INIT_AUTOMAKE(ec_slave, 2.0)
# Checks for programs.
AC_PROG_CC

# Checks for syslog
AC_ARG_WITH( syslog,
  AS_HELP_STRING([--with-syslog],[compile with syslog]),
  syslog=$withval, syslog=no )

if test x$syslog = xyes ; then
	AC_MSG_NOTICE([syslog enabled])
	AC_DEFINE(HAVE_SYSLOG)
else
	AC_MSG_NOTICE([syslog disabled])
fi

AC_CHECK_HEADERS([sys/socket.h])
AC_CHECK_HEADERS([stdlib.h])
AC_CHECK_HEADERS([arpa/inet.h])

AC_C_INLINE
AM_PROG_CC_C_O
AC_CHECK_FUNCS([socket])

AC_OUTPUT
