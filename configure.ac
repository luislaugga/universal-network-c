#
# Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
# Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
#
# universal-network-c
# configure.ac
#

# Include version m4 macro
m4_include([version.m4])

AC_PREREQ(2.50)
AC_INIT(backend, VERSION_NUMBER, [bugs@laugga.com])
AC_CONFIG_AUX_DIR(build)
AC_CONFIG_HEADER([config.h])

AC_CANONICAL_HOST
AC_CANONICAL_TARGET
AC_CANONICAL_BUILD

AC_PROG_CC([clang])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

# Checking for target system and setting custom configuration
# libdispatch and fblocks required in linux systems
case $target_os in
*darwin* | *rhapsody* | *macosx*)
     AC_DEFINE_UNQUOTED(DARWIN,1,[This is a Darwin system])
     ;;
*linux*)
     AC_DEFINE_UNQUOTED(LINUX,1,[This is a LINUX system])
	 AC_SEARCH_LIBS([dispatch_get_main_queue], [dispatch], [], [
	 	AC_MSG_ERROR([unable to find the dispatch_get_main_queue() function])
	 ])
	 CFLAGS="$CFLAGS -fblocks"
     ;;
esac 

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h stdlib.h sys/socket.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset socket strerror])

AC_CONFIG_FILES([Makefile
                 test/Makefile])
AC_OUTPUT