#ifndef __HDFS_H__
#define __HDFS_H__
/******************************************************************************/
/*                                                                            */
/*                       X r d H d f s . h h                        */
/*                                                                            */
/* (c) 2004 by the Board of Trustees of the Leland Stanford, Jr., University  */
/*       All Rights Reserved. See XrdInfo.cc for complete License Terms       */
/*   Produced by Andrew Hanushevsky for Stanford University under contract    */
/*              DE-AC03-76-SFO0515 with the Department of Energy              */
/******************************************************************************/

//         $Id: XrdHdfs.hh,v 1.15 2008/05/17 02:46:05 abh Exp $

#include <sys/types.h>
#include <string.h>
#include <dirent.h>
 
#include "XrdOuc/XrdOucErrInfo.hh"
#include "XrdOuc/XrdOucName2Name.hh"
#include "XrdOuc/XrdOucStream.hh"
 
#include "XrdOss/XrdOss.hh"
#include "XrdSfs/XrdSfsAio.hh"

#include "hdfs.h"


class XrdSfsAio;
class XrdSysLogger;

#define XrdHdfsMAX_PATH_LEN 1024
/******************************************************************************/
/*                 X r d H d f s D i r e c t o r y                  */
/******************************************************************************/
  
class XrdHdfsDirectory : public XrdOssDF
{
public:
        XrdOucErrInfo  error;

        int         Opendir(const char *dirName, XrdOucEnv &);
        int         Readdir(char *buff, int blen);
        int         Close(long long *retsz=0);

                    XrdHdfsDirectory(const char *tid=0);

                   ~XrdHdfsDirectory();

private:

hdfsFS        fs;
hdfsFileInfo  *dh;  // Directory handle
int            numEntries;
int            dirPos;
char          *fname;
const char    *tident;
int  isopen;
};

/******************************************************************************/
/*                      X r d H d f s F i l e                       */
/******************************************************************************/
  
class XrdHdfsFile : public XrdOssDF
{
public:

        XrdOucErrInfo  error;

        int            Open(const char                *fileName,
                                  int                  openMode,
                                  mode_t               createMode,
                                  XrdOucEnv           &client);

        int            Close(long long *retsz=0);

        int            Fstat(struct stat *);

        int            Fsync() {return -ENOTSUP;}

        int            Fsync(XrdSfsAio *a) {return -ENOTSUP;}

        off_t          getMmap(void **Addr)
                              {*Addr = 0; return 0;}

        ssize_t        Read(off_t               fileOffset,
                            size_t              preread_sz) {return XrdOssOK;}

        ssize_t        Read(void               *buffer,
                            off_t               fileOffset,
                            size_t              buffer_size);

        int            Read(XrdSfsAio *aioparm);

        ssize_t        ReadRaw(void            *buffer,
                            off_t               fileOffset,
                            size_t              buffer_size) {return Read(buffer, fileOffset, buffer_size);}

        ssize_t        Write(const char        *buffer,
                             off_t              fileOffset,
                             size_t             buffer_size);

        int            Write(XrdSfsAio *aioparm);

        int            Ftruncate(unsigned long long) {return -ENOTSUP;}

                       XrdHdfsFile(const char *user=0);

                       ~XrdHdfsFile();

private:

hdfsFS   fs; // File system object.
hdfsFile fh; // File Handle
char *fname; // File Name

char *readbuf;        // Read buffer
size_t readbuf_size;  // Memory allocated to readbuf
off_t readbuf_offset; // Offset in file of beginning of readbuf
size_t readbuf_len;   // Length of data last read into readbuf
unsigned int readbuf_bypassed;      // reads that were larger than readbuf
unsigned int readbuf_misses;        // reads for data not in readbuf
unsigned int readbuf_hits;          // reads satisfied by readbuf
unsigned int readbuf_partial_hits;  // reads partially satisfied by readbuf
unsigned long readbuf_bytes_used;   // extra bytes in readbuf that were eventually used
unsigned long readbuf_bytes_loaded; // extra bytes in readbuf that were read from disk

	// Although this class should not be assumed to be thread-safe,
	// for now, at least readbuf is protected by a mutex.  This
	// could eventually be applied more broadly.
XrdSysMutex readbuf_mutex;
};

/******************************************************************************/
/*                          X r d H d f s S y s                               */
/******************************************************************************/
  
class XrdHdfsSys : public XrdOss
{
public:
        XrdOucErrInfo  error;

// Object Allocation Functions
//
        XrdOssDF        *newDir(const char *user=0)
                        {return (XrdHdfsDirectory *)new XrdHdfsDirectory(user);}

        XrdOssDF        *newFile(const char *user=0)
                        {return      (XrdHdfsFile *)new XrdHdfsFile(user);}

// Other Functions
//
        int            Chmod(const char *, mode_t) {return -ENOTSUP;}
        int            Chmod(const char*, mode_t, XrdOucEnv*) {return -ENOTSUP;}

        int            Create(const char *, const char *, mode_t, XrdOucEnv &, int opts=0) {return -ENOTSUP;}

        int            Init(XrdSysLogger *, const char *);

        int            getStats(char *buff, int blen) {return 0;}

const   char          *getVersion();

        int            Mkdir(const char *, mode_t, int) {return -ENOTSUP;}
        int            Mkdir(const char*, mode_t, int, XrdOucEnv*) {return -ENOTSUP;}

        int            Remdir(const char *) {return -ENOTSUP;}
        int            Remdir(const char *, int) {return -ENOTSUP;}
        int            Remdir(const char*, int, XrdOucEnv*) {return -ENOTSUP;}

        int            Rename(const char *, const char *) {return -ENOTSUP;}
        int            Rename(const char*, const char*, XrdOucEnv*, XrdOucEnv*) {return -ENOTSUP;}

        int            Stat(const char *, struct stat *, int resonly=0, XrdOucEnv* env=0);

        int            Truncate(const char *, unsigned long long) {return -ENOTSUP;}
        int            Truncate(const char*, long long unsigned int, XrdOucEnv*) {return -ENOTSUP;}

        int            Unlink(const char *) {return -ENOTSUP;}
        int            Unlink(const char *, int) {return -ENOTSUP;}
        int            Unlink(const char*, int, XrdOucEnv*) {return -ENOTSUP;}

static  int            Emsg(const char *, XrdOucErrInfo&, int, const char *x,
                            const char *y="");

char             *N2N_Lib;   // -> Name2Name Library Path
char             *N2N_Parms; // -> Name2Name Object Parameters
XrdOucName2Name  *the_N2N;   // -> File mapper object
const char       *ConfigFN;  // Pointer to the configuration filename
XrdSysError      *eDest;

XrdHdfsSys() : XrdOss() {}
virtual ~XrdHdfsSys() {}

private:

int    Configure(const char *);
int    ConfigN2N();
int    ConfigProc(const char *);
int    ConfigXeq(char *, XrdOucStream &);
int    xnml(XrdOucStream &Config);


};
#endif
