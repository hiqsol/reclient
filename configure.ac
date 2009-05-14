# Process this file with autoconf to produce a configure script.

### Inits
AC_INIT(reClient, 0.1, bugs@reclient.com)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([reClient.cpp])
AM_CONFIG_HEADER([config.h])

### Checks for programs.
AC_PROG_CXX
AC_PROG_CC

### Checks for libraries.

### Checks for header files.
AC_HEADER_STDC

COMNETADDON_PATH="/usr/local"
AC_ARG_WITH(comnetaddon,	[  --with-comnetaddon      = DIR],
	COMNETADDON_PATH=$withval
)
AC_SUBST(EPPRTK_PATH)

EPPRTK_PATH="/usr/local/epp-rtk"
AC_ARG_WITH(epprtk,		[  --with-epprtk           = DIR],
	EPPRTK_PATH=$withval
)
AC_SUBST(EPPRTK_PATH)

DOMTOOLS_PATH="/usr/local"
AC_ARG_WITH(domtools,		[  --with-domtools-include = DIR],
    DOMTOOLS_PATH=$withval
)
AC_SUBST(DOMTOOLS_PATH)

XERCES_INCLUDE="/usr/local/include"
AC_ARG_WITH(xerces_include,	[  --with-xerces-include   = DIR],
    XERCES_INCLUDE=$withval
)
AC_SUBST(XERCES_INCLUDE)

XERCES_LIB="/usr/local/lib/libxerces-c1_7_0.so"
AC_ARG_WITH(xerces_lib,		[  --with-xerces-lib       = FILE],
    XERCES_LIB=$withval
)
AC_SUBST(XERCES_LIB)

OPENSSL_PATH="/usr/local/openssl"
AC_ARG_WITH(openssl,		[  --with-openssl          = DIR],
    OPENSSL_PATH=$withval
)
AC_SUBST(OPENSSL_PATH)

SSL_LIBS="-lssl -lcrypt -lcrypto"
AC_ARG_WITH(ssllibs,		[  --with-ssllibs          = DIR],
    SSL_LIBS=$withval
)
AC_SUBST(SSL_LIBS)

#AC_CHECK_HEADERS(epp-rtk-cpp/config.h epp-rtk-cpp/data/epp_eppdata.h,,
#	AC_MSG_ERROR("Missing header - probably missing epprtk -- looking for $EPPRTK_PATH/include/epp-rtk-cpp/config.h"))
#AC_CHECK_HEADERS(domtools/domtools.h,,
#	AC_MSG_ERROR("Missing header - probably missing domtools -- looking for $DOMTOOLS_PATH/include/domtools/domtools.h"))
#AC_CHECK_HEADERS(xercesc/dom/DOM.hpp xercesc/util/XMLString.hpp xercesc/util/XMLUniDefs.hpp,,
#	AC_MSG_ERROR("Missing header - probably missing Xerces -- looking for $XERCES_INCLUDE/xercesc/dom/DOM.hpp"))
#AC_CHECK_FILE($XERCES_LIB,,
#	AC_MSG_ERROR("Missing xerces shared lib: $XERCES_LIB! Use --with-xerces-lib to specify path to the lib"))

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.

# LLVM config
AC_SUBST(LLVM_CPPFLAGS, $(llvm-config --cppflags))
AC_SUBST(LLVM_LDADD, $(llvm-config --ldflags --libs core jit native ipo | tr '\n' ' '))

#CXXFLAGS="-Wall -I../.. -I/usr/local/include -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS"

CPPFLAGS="-Wall -g -I../.. -I$OPENSSL_PATH/include -I$XERCES_INCLUDE -I$DOMTOOLS_PATH/include -I$EPPRTK_PATH/include $LLVM_CPPFLAGS"

AC_OUTPUT(Makefile)
