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

LIBRARY =

TARGETS = OBJECTFILE

HADOOP_INCLUDE = -I$(HADOOP_HOME)/src/c++/libhdfs -I$(JAVA_HOME)/include
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

$(TARGETS): $(OBJECTS)
#	@echo Creating archive $(LIBRARY) 
#	@rm -f $(LIBRARY)
#	@ar -rc $(LIBRARY) $(OBJECTS)
#	@if [ "$(TYPE)" = "SunOS" -a "$(CC)" = "CC" ]; then \
#	@ar -rc $(LIBRARY) $(OBJDIR)$(SUNCACHE)/*.o; \
#fi

$(OBJDIR)/XrdHdfs.o:  XrdHdfsInterface.hh XrdHdfs.hh  XrdSecInterface.hh \
                           XrdSysError.hh     XrdOucErrInfo.hh XrdSysLogger.hh \
                           XrdHdfs.cc    XrdHdfsAio.hh  ../XrdVersion.hh
	@echo Compiling XrdHdfs.cc 
	$(ECHO)$(CC) -c $(CFLAGS) $(INCLUDE) $(HADOOP_INCLUDE) -o $(OBJDIR)/XrdHdfs.o XrdHdfs.cc
