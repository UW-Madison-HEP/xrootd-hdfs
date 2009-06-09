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
  
#include "XrdSfs/XrdSfsInterface.hh"

#include "hdfs.h"

class XrdSysError;
class XrdSysLogger;

/******************************************************************************/
/*                 X r d H d f s D i r e c t o r y                  */
/******************************************************************************/
  
class XrdHdfsDirectory : public XrdSfsDirectory
{
public:

        int         open(const char              *dirName,
                         const XrdSecClientName  *client = 0,
                         const char              *opaque = 0);

        const char *nextEntry();

        int         close();

const   char       *FName() {return (const char *)fname;}

                    XrdHdfsDirectory(char *user=0) : XrdSfsDirectory(user)
                                {
                                 const char * groups[1] = {"nobody"};
                                 fs = hdfsConnectAsUser("default", 0, user,
                                    groups, 1);
                                 dh  = (hdfsFileInfo*)NULL;
                                 numEntries = 0;
                                 dirPos = 0;
                                }

                   ~XrdHdfsDirectory() {if (dh != NULL && numEntries > 0) hdfsFreeFileInfo(dh, numEntries);}
private:

hdfsFS        fs;
hdfsFileInfo  *dh;  // Directory handle
int            numEntries;
int            dirPos;
char          *fname;

};

/******************************************************************************/
/*                      X r d H d f s F i l e                       */
/******************************************************************************/
  
class XrdHdfsFile : public XrdSfsFile
{
public:

        int            open(const char                *fileName,
                                  XrdSfsFileOpenMode   openMode,
                                  mode_t               createMode,
                            const XrdSecClientName    *client = 0,
                            const char                *opaque = 0);

        int            close();

        int            fctl(const int               cmd,
                            const char             *args,
                                  XrdOucErrInfo    &out_error);

        const char    *FName() {return fname;}

        int            getMmap(void **Addr, off_t &Size)
                              {if (Addr) Addr = 0; Size = 0; return SFS_OK;}

        int            read(XrdSfsFileOffset   fileOffset,
                            XrdSfsXferSize     preread_sz) {return SFS_OK;}

        XrdSfsXferSize read(XrdSfsFileOffset   fileOffset,
                            char              *buffer,
                            XrdSfsXferSize     buffer_size);

        int            read(XrdSfsAio *aioparm);

        XrdSfsXferSize write(XrdSfsFileOffset   fileOffset,
                             const char        *buffer,
                             XrdSfsXferSize     buffer_size);

        int            write(XrdSfsAio *aioparm);

        int            sync();

        int            sync(XrdSfsAio *aiop);

        int            stat(struct stat *buf);

        int            truncate(XrdSfsFileOffset   fileOffset);

        int            getCXinfo(char cxtype[4], int &cxrsz) {return cxrsz = 0;}

                       XrdHdfsFile(char *user=0) : XrdSfsFile(user)
                                          {
                                           const char * groups[1] = {"nobody"};
                                           fs = hdfsConnectAsUser("default", 0,
                                              user, groups, 1);
                                           fh = NULL; fname = 0;
                                          }

                      ~XrdHdfsFile() {if (fh) {hdfsCloseFile(fs, fh); fh=NULL;}}
private:

hdfsFS   fs; // File system object.
hdfsFile fh; // File Handle
char *fname; // File Name

};

/******************************************************************************/
/*                          X r d H d f s                           */
/******************************************************************************/
  
class XrdHdfs : public XrdSfsFileSystem
{
public:

// Object Allocation Functions
//
        XrdSfsDirectory *newDir(char *user=0)
                        {return (XrdSfsDirectory *)new XrdHdfsDirectory(user);}

        XrdSfsFile      *newFile(char *user=0)
                        {return      (XrdSfsFile *)new XrdHdfsFile(user);}

// Other Functions
//
        int            chmod(const char             *Name,
                                   XrdSfsMode        Mode,
                                   XrdOucErrInfo    &out_error,
                             const XrdSecClientName *client = 0,
                             const char             *opaque = 0);

        int            exists(const char                *fileName,
                                    XrdSfsFileExistence &exists_flag,
                                    XrdOucErrInfo       &out_error,
                              const XrdSecClientName    *client = 0,
                              const char                *opaque = 0);

        int            fsctl(const int               cmd,
                             const char             *args,
                                   XrdOucErrInfo    &out_error,
                             const XrdSecClientName *client = 0);

        int            getStats(char *buff, int blen) {return 0;}

const   char          *getVersion();

        int            mkdir(const char             *dirName,
                                   XrdSfsMode        Mode,
                                   XrdOucErrInfo    &out_error,
                             const XrdSecClientName *client = 0,
                             const char             *opaque = 0);

        int            prepare(      XrdSfsPrep       &pargs,
                                     XrdOucErrInfo    &out_error,
                               const XrdSecClientName *client = 0) {return 0;}

        int            rem(const char             *path,
                                 XrdOucErrInfo    &out_error,
                           const XrdSecClientName *client = 0,
                           const char             *opaque = 0);

        int            remdir(const char             *dirName,
                                    XrdOucErrInfo    &out_error,
                              const XrdSecClientName *client = 0,
                              const char             *opaque = 0);

        int            rename(const char             *oldFileName,
                              const char             *newFileName,
                                    XrdOucErrInfo    &out_error,
                              const XrdSecClientName *client = 0,
                              const char             *opaqueO = 0,
                              const char             *opaqueN = 0);

        int            stat(const char             *Name,
                                  struct stat      *buf,
                                  XrdOucErrInfo    &out_error,
                            const XrdSecClientName *client = 0,
                            const char             *opaque = 0);

        int            stat(const char             *Name,
                                  mode_t           &mode,
                                  XrdOucErrInfo    &out_error,
                            const XrdSecClientName *client = 0,
                            const char             *opaque = 0)
                       {struct stat bfr;
                        int rc = stat(Name, &bfr, out_error, client);
                        if (!rc) mode = bfr.st_mode;
                        return rc;
                       }

        int            truncate(const char             *Name,
                                      XrdSfsFileOffset fileOffset,
                                      XrdOucErrInfo    &out_error,
                                const XrdSecEntity     *client = 0,
                                const char             *opaque = 0);

// Common functions
//
static  int            Mkpath(const char *path, mode_t mode, 
                              const char *info=0);

static  int            Emsg(const char *, XrdOucErrInfo&, int, const char *x,
                            const char *y="");

                       XrdHdfs(XrdSysError *lp);
virtual               ~XrdHdfs() {}

private:

static XrdSysError *eDest;

};
#endif
