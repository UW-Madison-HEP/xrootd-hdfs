#######################################################################
## Makefile.am for xrootd
##
##
## Initial version: 1.8.2005
##
## Version info: $Id: Makefile.am,v 1.2 2007/06/07 10:45:05 dfeich Exp $
## Checked in by $Author: dfeich $
#######################################################################

lib_LTLIBRARIES = libXrdHdfs.la

xrootdsfsincdir = $(includedir)/xrootd/XrdHdfs

xrootdsfsinc_HEADERS = src/XrdHdfs.hh

INCLUDES = -I$(top_srcdir)/src

libXrdHdfs_la_SOURCES = \
         src/XrdHdfs.cc src/XrdHdfsConfig.cc src/XrdHdfs.hh

libXrdHdfs_la_LDFLAGS = -lhdfs

EXTRA_DIST = spec/xrootd-hdfs.spec spec/xrootd.sample.hdfs.cfg.in spec/xrootd-hdfs.sysconfig

