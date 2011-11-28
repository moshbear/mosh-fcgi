AC_INIT([mosh_fcgi], [0.4r2], [m0shbear <andrey@moshbear.net>], [mosh_fcgi])
AC_CONFIG_AUX_DIR(m4)
AC_CONFIG_FILES(mosh_fcgi.pc)
AC_CONFIG_HEADERS([include/mosh/fcgi/bits/config.h])
AM_INIT_AUTOMAKE(1.9 dist-bzip2 no-dist-gzip tar-ustar foreign no-dependencies)

AC_LANG(C++)
AC_REQUIRE_CPP

AC_PROG_INSTALL
AC_PROG_LIBTOOL

BOOST_BIND
BOOST_DATE_TIME
BOOST_FUNCTION
BOOST_REGEX
BOOST_SMART_PTR
BOOST_THREADS
BOOST_TOKENIZER
BOOST_SYSTEM

PKG_CHECK_MODULES([NSS], [nss], [have_nss=yes],
	[AC_SEARCH_LIBS([HASH_Create], [nss nss3],
		    [have_nss=yes],
		    [true],
		    [-lpthread])
])
AC_SUBST([NSS_CFLAGS], [$NSS_CFLAGS])
AC_SUBST([NSS_LIBS], [$NSS_LIBS])

AC_PROG_CXX

CXX="${CXX} -std=c++0x"

AC_MSG_NOTICE([checking for language features])
AC_COMPILE_CHECK([c++11 angle brackets], [ template <typename T> class Q { }; ] , [ Q<Q<int>> foo; ], [AC_MSG_NOTICE([working])], [AC_MSG_ERROR([no improved angle bracket support])])
AC_COMPILE_CHECK([c++11 auto], , [ int foo; auto bar = foo; ], [AC_MSG_NOTICE([working])], [AC_MSG_ERROR([no auto support])])
AC_COMPILE_CHECK([c++11 rvalue reference], [int p() { return 0; } ] , [ int&& q = p(); ], [AC_MSG_NOTICE([working])], [AC_MSG_ERROR([no rvalue reference support])])
AC_COMPILE_CHECK([c++11 lambda], , [ auto&& foo = [] (int x) { return x; }; ], [AC_MSG_NOTICE([working])], [AC_MSG_ERROR([no lambda support])])
AC_COMPILE_CHECK([c++11 nullptr], , [ void *x = nullptr; ], [AC_MSG_NOTICE([working])], [AC_MSG_ERROR([no nullptr support])]) 
AC_COMPILE_CHECK([c++11 strongly-typed enum], [ enum class foo : char { v0 = 0, v1 = 1}; ], [ foo f; ], [AC_MSG_NOTICE([working])],  [AC_MSG_ERROR([no strongly-typed enum support])])
dnl todo: rb-for workaround where applicable
AC_COMPILE_CHECK([c++11 range-based for], , [ int foo[2] = {0, 1}; for (auto& bar : foo); ], [AC_MSG_NOTICE([working])], [AC_MSG_ERROR([no range-based for support])])
AC_MSG_NOTICE([checking for library features])
AC_COMPILE_CHECK([<cstdint>], [#include <cstdint>], , [AC_MSG_NOTICE([yes])], [AC_MSG_ERROR([no <cstdint>])])
AC_COMPILE_CHECK([c++11 smart pointer], [#include <memory>], [ std::unique_ptr<void> p; std::shared_ptr<void> q; ], [AC_MSG_NOTICE([working])],[AC_MSG_ERROR([no unique_ptr])])
AC_COMPILE_CHECK([c++11 fstream], [#include <iostream>
					#include <fstream>
					#include <utility>],
				[ std::ifstream a; std::ifstream b; b = std::move(a); ],
				[AC_DEFINE([HAVE_CXX11_FSTREAM], [1], [Define if std::fstream and related classes are C++11-compliant])],
				[AC_MSG_NOTICE([not compliant; using smart_ptr workaround])]
		)

AC_COMPILE_CHECK([<poll.h>], [#include <poll.h>], ,
		[	AC_MSG_NOTICE([yes])
		 	AC_DEFINE([HAVE_POLL_H], [1], [Defined if <poll.h> exists])
		], [AC_MSG_ERROR([no <poll.h>])])

AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

pkgConfigLibs="-lmosh_fcgi $BOOST_THREAD_LIBS $BOOST_REGEX_LIBS $BOOST_DATE_TIME_LIBS $CRYPTOPP_LIBS"

AC_SUBST(pkgConfigLibs)

AC_OUTPUT([Makefile \
	src/Makefile \
	include/Makefile \
	examples/Makefile])
