
AC_PREREQ(2.57)
AC_INIT(xrootd-hdfs,1.4.6,xrootd-demonstrator@opensciencegrid.org)
AC_CONFIG_SRCDIR([src/XrdHdfs.hh])

AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL

AC_LANG_CPLUSPLUS

AM_INIT_AUTOMAKE

AC_ARG_WITH(xrootd-libdir,
            [  --with-xrootd-libdir=PATH        path to xrootd libraries],
            XROOTD_LIBDIR=-L$withval)
AC_SUBST(XROOTD_LIBDIR)
AC_ARG_WITH(xrootd-incdir,
            [  --with-xrootd-incdir=PATH        path to xrootd header files],
            XROOTD_INCDIR=-I$withval)
AC_SUBST(XROOTD_INCDIR)

AC_ARG_WITH(hdfs-libdir,
            [  --with-hdfs-libdir=PATH        directory path to libhdfs.so library],
            HDFS_LIBDIR=-L$withval)
AC_SUBST(HDFS_LIBDIR)
AC_ARG_WITH(hdfs-incdir,
            [  --with-hdfs-incdir=PATH        directory path to hdfs.h header file],
            HDFS_INCDIR=-I$withval)
AC_SUBST(HDFS_INCDIR)

AC_ARG_WITH(jvm-libdir,
            [  --with-jvm-libdir=PATH         directory path to libjvm.so library],
            JVM_LIBDIR=-L$withval)
AC_SUBST(JVM_LIBDIR)
AC_ARG_WITH(jvm-incdir,
            [  --with-jvm-incdir=PATH        directory path to jni.h header file],
            [JVM_INCDIR="-I$withval -I$withval/linux"])
AC_SUBST(JVM_INCDIR)

# determines shared library needed for loading modules
AC_LIBTOOL_DLOPEN
# init for libtool
AC_PROG_LIBTOOL

AM_PROG_CC_C_O

# libhdfs.so
LDFLAGS="$JVM_LIBDIR $HDFS_LIBDIR $XROOTD_LIBDIR $LDFLAGS"
AC_CHECK_LIB([hdfs],[hdfsExists],[:],
             AC_MSG_ERROR([Failed linking test for libhdfs.so]), [-ljvm]
            )
LDFLAGS=$LDFLAGS_BUP

CXXFLAGS="$HDFS_INCDIR $XROOTD_INCDIR $JVM_INCDIR $INCLUDES"
AC_CHECK_HEADER([hdfs.h])
AC_CHECK_HEADER([jni.h])

CXXFLAGS="$CXXFLAGS -D_REENTRANT -D_FILE_OFFSET_BITS=64"

AC_OUTPUT(Makefile)

