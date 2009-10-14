#              $Id: GNUmakefile,v 1.10 2007/07/31 02:26:31 abh Exp $

#------------------------------------------------------------------------------#
#                       C o m m o n   V a r i a b l e s                        #
#------------------------------------------------------------------------------#
  
include ../GNUmake.env

#------------------------------------------------------------------------------#
#                             C o m p o n e n t s                              #
#------------------------------------------------------------------------------#
  
SOURCES = \
        XrdHdfs.cc

  
OBJECTS = \
        $(OBJDIR)/XrdHdfs.o

LIBARCH  = $(LIBDIR)/libXrdHdfs.a
LIBRARY  = $(LIBDIR)/libXrdHdfs.$(TYPESHLIB)

TARGETS = OBJECTFILE $(LIBARCH) $(LIBRARY)

HADOOP_INCLUDE = -I$(HADOOP_HOME)/src/c++/libhdfs -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/linux

HADOOP_LIB = -L$(HADOOP_HOME)/build/libhdfs -lhdfs

#------------------------------------------------------------------------------#
#                           S e a r c h   P a t h s                            #
#------------------------------------------------------------------------------#

vpath XrdOuc% ../XrdOuc
vpath XrdSec% ../XrdSec
vpath XrdSys% ../XrdSys

#------------------------------------------------------------------------------#
#                          I n i t i a l   R u l e s                           #
#------------------------------------------------------------------------------#
 
include ../GNUmake.options

anything: $(TARGETS)
	@echo Make XrdHdfs done.

#------------------------------------------------------------------------------#
#                           D e p e n d e n c i e s                            #
#------------------------------------------------------------------------------#

$(TARGETS): $(OBJECTS) $(OBJFS) $(LIBDEP)
	@echo Creating archive $(LIBARCH)
	$(ECHO)rm -f $(LIBARCH)
	$(ECHO)if [ "$(TYPE)" = "SunOS" -a "$(CC)" = "CC" ]; then \
                  if [ "x$(SUNCACHE)" != "x" ]; then \
                     $(CC) -xar -o $(LIBARCH) $(OBJDIR)$(SUNCACHE)/*/*.o; \
                  else \
                     $(CC) -xar -o $(LIBARCH) $(OBJECTS); \
                  fi; \
               else \
                  ar -rc $(LIBARCH) $(OBJECTS); \
                  ranlib $(LIBARCH); \
               fi
	@echo Creating shared library $(LIBRARY) 
	$(ECHO)$(CC) $(CFLAGS) $(OBJECTS) $(OBJFS) $(LDSO) $(MORELIBS) $(LIBS) $(HADOOP_LIB) -o $(LIBRARY)

$(OBJDIR)/XrdHdfs.o:  XrdHdfs.hh  \
                           XrdSysError.hh     XrdOucErrInfo.hh XrdSysLogger.hh \
                           XrdHdfs.cc    ../XrdVersion.hh
	@echo Compiling XrdHdfs.cc 
	$(ECHO)$(CC) -c $(CFLAGS) $(INCLUDE) $(HADOOP_INCLUDE) -o $(OBJDIR)/XrdHdfs.o XrdHdfs.cc

