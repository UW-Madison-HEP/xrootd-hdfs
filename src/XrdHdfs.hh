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

namespace XrdHdfs
{
    class ChecksumState;
}

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
        int         StatRet(struct stat *buff);

                    XrdHdfsDirectory(const char *tid=0);

                   ~XrdHdfsDirectory();

private:

hdfsFS        fs;
hdfsFileInfo  *dh;  // Directory handle
int            numEntries;
int            dirPos;
char          *fname;
const char    *tident;
bool           isopen;

// A pointer to the current entry in the directory; if set non-null, we
// automatically populate this with the entry's "Stat" information.  This allows
// Xrootd to save a corresponding call to `Stat` for each directory entry.
struct stat *m_stat_buf;
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

        ssize_t        Write(const void *buffer,
                             off_t              fileOffset,
                             size_t             buffer_size);

        int            Write(XrdSfsAio *aioparm);

        int            Ftruncate(unsigned long long) {return -ENOTSUP;}

        XrdHdfsFile(const char *user=0);

        ~XrdHdfsFile();

private:

hdfsFS   m_fs; // File system object.
hdfsFile fh; // File Handle
char *fname; // File Name
ssize_t m_nextoff; // Next offset for writes.

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

        // Keep track of checksum values for files that are being written.
    XrdHdfs::ChecksumState *m_state;

    bool Connect(const XrdOucEnv &);
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
        int            Chmod(const char*, mode_t, XrdOucEnv* envp=NULL);

        int            Create(const char *, const char *, mode_t, XrdOucEnv &,
                              int opts=0);

        uint64_t       Features()  // Turn async I/O off for hdfs
                       {return XRDOSS_HASNAIO;}

        int            Init(XrdSysLogger *, const char *);

        int            getStats(char *buff, int blen) {return 0;}

const   char          *getVersion();

        int            Mkdir(const char *path, mode_t mode, int mkpath=0,
                             XrdOucEnv *envp=NULL);

        int            Remdir(const char*, int Opts=0, XrdOucEnv *envp=NULL);

        int            Rename(const char*, const char*,
                              XrdOucEnv *envp_src=NULL,
                              XrdOucEnv *envp_dest=NULL);

        int            Stat(const char *, struct stat *, int resonly=0, XrdOucEnv* env=0);

        int            Truncate(const char*, long long unsigned int,
                                XrdOucEnv *envp=NULL);

        int            Unlink(const char*, int Opts=0, XrdOucEnv *envp=NULL);

static  int            Emsg(const char *, XrdOucErrInfo&, int, const char *x,
                            const char *y="");

        void           Say(const char *, const char *x="", const char *y="",
                           const char *z="");

char * GetRealPath(const char *);  // Given a requested pathname, translate it to the HDFS path.  Caller must free() returned space.

virtual int            Lfn2Pfn(const char *Path, char *buff, int blen);
virtual const char    *Lfn2Pfn(const char *Path, char *buff, int blen, int &rc);

XrdHdfsSys() : XrdOss() {}
virtual ~XrdHdfsSys() {}

private:
char             *N2N_Lib;   // -> Name2Name Library Path
char             *N2N_Parms; // -> Name2Name Object Parameters
XrdOucName2Name  *the_N2N;   // -> File mapper object
const char       *ConfigFN;  // Pointer to the configuration filename
XrdSysError      *eDest;

int    Configure(const char *);
int    ConfigN2N();
int    ConfigProc(const char *);
int    ConfigXeq(char *, XrdOucStream &);
int    xnml(XrdOucStream &Config);

// Static instance of the HDFS filesystem; this is used by the cmsd in order
// to avoid opening / closing the filesystem repeatedly (reduces the number of
// new connections to the namenode).
static XrdSysMutex m_fs_mutex;
static hdfsFS m_cmsd_fs;

};
#endif
