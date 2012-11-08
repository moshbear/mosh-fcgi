AC_INIT([mosh_fcgi], [0.6.0], [m0shbear <andrey@moshbear.net>], [mosh_fcgi])
AC_CONFIG_AUX_DIR(m4)
AC_CONFIG_FILES(mosh_fcgi.pc)
AC_CONFIG_HEADERS([include/mosh/fcgi/bits/config.h])
AM_INIT_AUTOMAKE(1.9 dist-bzip2 no-dist-gzip tar-ustar foreign no-dependencies)

AC_LANG(C++)
AC_REQUIRE_CPP

AC_PROG_INSTALL
AC_PROG_LIBTOOL

BOOST_XPRESSIVE
BOOST_THREADS

AC_PROG_CXX

CXX="${CXX} -std=c++0x -Wall -g -ggdb -O2"

AC_MSG_NOTICE([checking for language features])
AC_COMPILE_CHECK([c++11 angle brackets], [ template <typename T> class Q { }; ] , [ Q<Q<int>> foo; ], [AC_MSG_NOTICE([working])], [AC_MSG_ERROR([no improved angle bracket support])])
AC_COMPILE_CHECK([c++11 auto], , [ int foo; auto bar = foo; ], [AC_MSG_NOTICE([working])], [AC_MSG_ERROR([no auto support])])
AC_COMPILE_CHECK([c++11 rvalue reference], [int p() { return 0; } ] , [ int&& q = p(); ], [AC_MSG_NOTICE([working])], [AC_MSG_ERROR([no rvalue reference support])])
AC_COMPILE_CHECK([c++11 lambda], , [ auto&& foo = [] (int x) { return x; }; ], [AC_MSG_NOTICE([working])], [AC_MSG_ERROR([no lambda support])])
AC_COMPILE_CHECK([c++11 array], [#include <array> ], [std::array<int, 1> x; ], [AC_MSG_NOTICE([working])],  [AC_MSG_ERROR([no c++11 array])])
AC_COMPILE_CHECK([c++11 nullptr], , [ void *x = nullptr; ], [AC_MSG_NOTICE([working])], [AC_MSG_ERROR([no nullptr support])]) 
AC_COMPILE_CHECK([c++11 strongly-typed enum], [ enum class foo : char { v0 = 0, v1 = 1}; ], [ foo f; ], [AC_MSG_NOTICE([working])],  [AC_MSG_ERROR([no strongly-typed enum support])])
dnl todo: rb-for workaround where applicable
AC_COMPILE_CHECK([c++11 range-based for], , [ int foo[2] = {0, 1}; for (auto& bar : foo); ], [AC_MSG_NOTICE([working])], [AC_MSG_ERROR([no range-based for support])])
AC_MSG_NOTICE([checking for library features])
AC_COMPILE_CHECK([<cstdint>], [#include <cstdint>], , [AC_MSG_NOTICE([yes])], [AC_MSG_ERROR([no <cstdint>])])
AC_COMPILE_CHECK([c++11 smart pointer], [#include <memory>], [ std::unique_ptr<void> p; std::shared_ptr<void> q; ], [AC_MSG_NOTICE([working])],[AC_MSG_ERROR([no c++11 smart_ptr])])
AC_COMPILE_CHECK([c++11 <mutex>::call_once], [#include <mutex>], [ std::once_flag o; ], [AC_MSG_NOTICE([working])], [AC_MSG_ERROR([no <mutex>])])
AC_COMPILE_CHECK([<poll.h>], [#include <poll.h>], ,
		[	AC_MSG_NOTICE([yes])
		 	AC_DEFINE([HAVE_POLL_H], [1], [Defined if <poll.h> exists])
		], [AC_MSG_ERROR([no <poll.h>])])

AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

pkgConfigLibs="-lmosh_fcgi"

AC_SUBST(pkgConfigLibs)

AC_OUTPUT([Makefile \
	src/Makefile \
	include/Makefile \
	examples/Makefile \
	examples/echo/Makefile \
	examples/filter/Makefile \
	examples/raw-echo/Makefile \
	examples/show-gnu/Makefile \
	examples/timer/Makefile \
	examples/upload/Makefile \
	examples/utf8-helloworld/Makefile])
