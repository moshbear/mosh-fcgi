SUBDIRS = include src examples

DISTCLEANFILES = Makefile Makefile.in

EXTRA_DIST = LICENSE_1_0.txt

nobase_dist_pkgdata_DATA = doc/*

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = mosh_fcgi.pc

doc/*:
	doxygen

