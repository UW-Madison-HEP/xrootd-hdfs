/******************************************************************************/
/*                                                                            */
/*                       X r d H d f s . c c                                  */
/*                                                                            */
/* (c) 2004 by the Board of Trustees of the Leland Stanford, Jr., University  */
/*       All Rights Reserved. See XrdInfo.cc for complete License Terms       */
/*   Produced by Andrew Hanushevsky for Stanford University under contract    */
/*               DE-AC03-76-SFO0515 with the Deprtment of Energy              */
/******************************************************************************/

const char *XrdHdfsSVNID = "$Id$";

#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <memory.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/stat.h>

#include "XrdVersion.hh"
#include "XrdSys/XrdSysError.hh"
#include "XrdSys/XrdSysHeaders.hh"
#include "XrdSys/XrdSysLogger.hh"
#include "XrdSys/XrdSysPlugin.hh"
#include "XrdSys/XrdSysPthread.hh"
#include "XrdSec/XrdSecInterface.hh"
#include "XrdOss/XrdOssTrace.hh"
#include "XrdHdfs.hh"

#ifdef AIX
#include <sys/mode.h>
#endif

/******************************************************************************/
/*       O S   D i r e c t o r y   H a n d l i n g   I n t e r f a c e        */
/******************************************************************************/

#ifndef S_IAMB
#define S_IAMB  0x1FF
#endif

/******************************************************************************/
/*                  E r r o r   R o u t i n g   O b j e c t                   */
/******************************************************************************/

namespace XrdHdfs
{

XrdSysError OssEroute(0, "hdfs_");

XrdOucTrace OssTrace(&OssEroute);

static XrdHdfsSys XrdHdfsSS;

}

using namespace std;
using namespace XrdHdfs;

/******************************************************************************/
/*                         G e t F i l e S y s t e m                          */
/******************************************************************************/
extern "C"
{
XrdOss *XrdOssGetStorageSystem(XrdOss       *native_oss,
                               XrdSysLogger *Logger,
                         const char         *config_fn,
                         const char         *parms)
{
   return (XrdHdfsSS.Init(Logger, config_fn) ? 0 : (XrdOss *)&XrdHdfsSS);
}
}

/******************************************************************************/
/*           D i r e c t o r y   O b j e c t   I n t e r f a c e s            */
/******************************************************************************/
/******************************************************************************/
/*                                  O p e n d i r                             */
/******************************************************************************/
  
int XrdHdfsDirectory::Opendir(const char              *dir_path)
/*
  Function: Open the directory `path' and prepare for reading.

  Input:    path      - The fully qualified name of the directory to open.

  Output:   Returns XrdOssOK upon success, otherwise (-errno).
*/
{
#ifndef NODEBUG
   static const char *epname = "Opendir";
#endif
   int retc;

// Return an error if we have already opened
   if (isopen) return -XRDOSS_E8001;

// Set up values for this directory object
//
   if (XrdHdfsSS.the_N2N) {
       char actual_path[XrdHdfsMAX_PATH_LEN+1];
       if ((retc = (XrdHdfsSS.the_N2N)->lfn2pfn(dir_path, actual_path, sizeof(actual_path)))) {
          (XrdHdfsSS.eDest)->Say("Cannot find a N2N mapping for ", dir_path, "; using path directly.");
          fname = strdup(dir_path);
       }
          else fname = strdup(actual_path);
   } else {
       fname = strdup(dir_path);
   }
   dirPos = 0;

// Open the directory and get it's id
//
   TRACE(Opendir, "path " << fname);
   if (!(dh = hdfsListDirectory(fs, fname, &numEntries))) {
      isopen = 0;
      return (errno <= 0) ? -1 : -errno;
   }
   isopen = 1;

// All done
//
   return XrdOssOK;
}

/******************************************************************************/
/*                             R e a d d i r                                  */
/******************************************************************************/

int XrdHdfsDirectory::Readdir(char * buff, int blen)
/*
  Function: Read the next directory entry.

  Input:    None.

  Output:   Upon success, returns the contents of the next directory entry as
            a null terminated string. Returns a null pointer upon EOF or an
            error. To differentiate the two cases, getErrorInfo will return
            0 upon EOF and an actual error code (i.e., not 0) on error.
*/
{
#ifndef NODEBUG
    static const char *epname = "Readdir";
#endif

  if (!isopen) return -XRDOSS_E8002;

// Lock the direcrtory and do any required tracing
//
  if (!dh)  {
     XrdHdfsSys::Emsg(epname,error,EBADF,"read directory",fname);
     return -EBADF;
  }

// Check if we are at EOF (once there we stay there)
//
   if (dirPos == numEntries) {
     *buff = '\0';
     return 0;
   }

// Read the next directory entry
//
   hdfsFileInfo fileInfo = dh[dirPos];
   dirPos++;

// Return the actual entry
//
   std::string full_name = fileInfo.mName;
   full_name.erase(0, full_name.rfind("/"));
   strlcpy(buff, full_name.c_str(), blen);
   return XrdOssOK;
}

/******************************************************************************/
/*                                 C l o s e                                  */
/******************************************************************************/
  
int XrdHdfsDirectory::Close(long long *retsz)
/*
  Function: Close the directory object.

  Input:    cred       - Authentication credentials, if any.

  Output:   Returns XrdOssOK upon success and XRDOSS_E8002 upon failure.
*/
{

   if (!isopen) return -XRDOSS_E8002;

// Release the handle
//
   if (dh != NULL && numEntries >= 0) {
      hdfsFreeFileInfo(dh, numEntries);
   }

// Do some clean-up
//
   if (fname) {
     free(fname);
     fname = NULL;
   }
   dh = (hdfsFileInfo *)0; 
   numEntries = 0;
   dirPos = 0;
   isopen = 0;
   return 0;
}

XrdHdfsDirectory::~XrdHdfsDirectory()
{
  if (dh != NULL && numEntries >= 0) {
    hdfsFreeFileInfo(dh, numEntries);
  }
  if (fs != NULL) {
    hdfsDisconnect(fs);
  }
  if (fname) {
    free(fname);
  }
}

/******************************************************************************/
/*                F i l e   O b j e c t   I n t e r f a c e s                 */
/******************************************************************************/


/******************************************************************************/
/*                          C o n s t r u c t o r                             */
/******************************************************************************/
XrdHdfsFile::XrdHdfsFile(const char *user) : XrdOssDF(), fh(NULL), fname(NULL)
{
  fs = hdfsConnectAsUserNewInstance("default", 0,
    "nobody");
}

/******************************************************************************/
/*                                  O p e n                                   */
/******************************************************************************/

int XrdHdfsFile::Open(const char               *path,      // In
                            int                 openMode,  // In
                            mode_t              createMode,// In
                            XrdOucEnv          &client)    // In
/*
  Function: Open the file `path' in the mode indicated by `openMode'.  

  Input:    path      - The fully qualified name of the file to open.
            openMode  - One of the following flag values:
                        SFS_O_RDONLY - Open file for reading.
                        SFS_O_WRONLY - Open file for writing.
                        SFS_O_RDWR   - Open file for update
                        SFS_O_CREAT  - Create the file open in RDWR mode
                        SFS_O_TRUNC  - Trunc  the file open in RDWR mode
            Mode      - The Posix access mode bits to be assigned to the file.
                        These bits correspond to the standard Unix permission
                        bits (e.g., 744 == "rwxr--r--"). Mode may also conatin
                        SFS_O_MKPTH is the full path is to be created. The
                        agument is ignored unless openMode = O_CREAT.
            client    - Authentication credentials, if any.
            info      - Opaque information to be used as seen fit.

  Output:   Returns OOSS_OK upon success, otherwise -errno is returned.
*/
{
#ifndef NODEBUG
   static const char *epname = "open";
#endif
   //const int AMode = S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH; // 775
   char *opname;
   int open_flag = 0;

// Verify that this object is not already associated with an open file
//
   if (fh != NULL)
      return -XRDOSS_E8003;

   int retc;
   if (XrdHdfsSS.the_N2N) {
       char actual_path[XrdHdfsMAX_PATH_LEN+1];
       if ((retc = (XrdHdfsSS.the_N2N)->lfn2pfn(path, actual_path, sizeof(actual_path)))) {
          (XrdHdfsSS.eDest)->Say("Cannot find a N2N mapping for ", path, "; using path directly.");
          fname = strdup(path);
       }
       else fname = strdup(actual_path);
   } else {
       fname = strdup(path);
   }

   (XrdHdfsSS.eDest)->Say("File we will access: ", fname);

// Set the actual open mode
//
   switch(openMode & (O_RDONLY | O_WRONLY | O_RDWR))
   {
   case O_RDONLY: open_flag = O_RDONLY; break;
   case O_WRONLY: open_flag = O_WRONLY; break;
   case O_RDWR:   open_flag = O_RDWR;   break;
   default:           open_flag = O_RDONLY; break;
   }

   // HDFS does not support read-write mode.
   if (openMode & O_RDWR) {
       return XrdHdfsSys::Emsg(epname,error,ENOTSUP, "Read-write mode not"
           " supported by HDFS.",path);
   }

// Prepare to create or open the file, as needed
//
   if (openMode & O_CREAT) {
       opname = (char *)"create";
   } else if (openMode & O_TRUNC) {
         open_flag  = O_TRUNC;
                 opname = (char *)"truncate";
      } else opname = (char *)"open";

// Open the file and make sure it is a file
//

   int err_code = 0;

   if ((fh = hdfsOpenFile(fs, fname, open_flag, 0, 0, 0)) == NULL) {
       err_code = errno;
       hdfsFileInfo * fileInfo = hdfsGetPathInfo(fs, fname);
       if (fileInfo != NULL) {
           if (fileInfo->mKind == kObjectKindDirectory) {
                   err_code = EISDIR;
           }
           hdfsFreeFileInfo(fileInfo, 1);
       } else { 
           err_code = ENOENT;
       }
   }

// All done.
//
   if (err_code != 0)
       return (err_code > 0) ? -err_code : err_code;

   return XrdOssOK;
}

/******************************************************************************/
/*                                 C l o s e                                  */
/******************************************************************************/

int XrdHdfsFile::Close(long long int *)
/*
  Function: Close the file object.

  Input:    None

  Output:   Returns XrdOssOK upon success and -errno upon failure.
*/
{
   static const char *epname = "close";

// Release the handle and return
//
   int ret = XrdOssOK;
   if (fh != NULL  && hdfsCloseFile(fs, fh) != 0) {
      ret = XrdHdfsSys::Emsg(epname, error, errno, "close", fname);
   }
   fh = NULL;
   if (fs != NULL && hdfsDisconnect(fs) != 0) {
      ret = XrdHdfsSys::Emsg(epname, error, errno, "close", fname); 
   }
   fs = NULL;
   if (fname) {
      free(fname);
      fname = 0;
   }
   return ret;
}

XrdHdfsFile::~XrdHdfsFile()
{
   if (fs && fh) {hdfsCloseFile(fs, fh);}
   if (fs) {hdfsDisconnect(fs);}
   if (fname) {free(fname);}
}
  
/******************************************************************************/
/*                                  R e a d                                   */
/******************************************************************************/

ssize_t XrdHdfsFile::Read(void   *buff,      // Out
                          off_t   offset,    // In
                          size_t  blen)      // In
/*
  Function: Read `blen' bytes at `offset' into 'buff' and return the actual
            number of bytes read.

  Input:    offset    - The absolute byte offset at which to start the read.
            buff      - Address of the buffer in which to place the data.
            blen      - The size of the buffer. This is the maximum number
                        of bytes that will be read from 'fd'.

  Output:   Returns the number of bytes read upon success and -errno o/w.
*/
{
#ifndef NODEBUG
   static const char *epname = "Read";
#endif
   size_t nbytes;

// Read the actual number of bytes
//
   nbytes = hdfsPread(fs, fh, (off_t)offset, (void *)buff, (size_t)blen);

   if (nbytes  < 0)
      return XrdHdfsSys::Emsg(epname, error, errno, "read", fname);

// Return number of bytes read
//
   return nbytes;
}
  
/******************************************************************************/
/*                              R e a d   A I O                               */
/******************************************************************************/
  
int XrdHdfsFile::Read(XrdSfsAio *aiop)
{

// Execute this request in a synchronous fashion
//
   aiop->Result = this->Read((void *)aiop->sfsAio.aio_buf, aiop->sfsAio.aio_offset,
                             aiop->sfsAio.aio_nbytes);
   aiop->doneRead();
   return 0;
}

/******************************************************************************/
/*                                 W r i t e                                  */
/******************************************************************************/

ssize_t XrdHdfsFile::Write(const char *buff,    // In
                                 off_t offset,  // In
                                 size_t blen)   // In
/*
  Function: Write `blen' bytes at `offset' from 'buff' and return the actual
            number of bytes written.

  Input:    offset    - The absolute byte offset at which to start the write.
            buff      - Address of the buffer from which to get the data.
            blen      - The size of the buffer. This is the maximum number
                        of bytes that will be written to 'fd'.

  Output:   Returns the number of bytes written upon success and -errno o/w.

  Notes:    An error return may be delayed until the next write(), close(), or
            sync() call.

This is currently not implemented for HDFS

*/
{
   static const char *epname = "write";

// Make sure the offset is not too large
//
#if _FILE_OFFSET_BITS!=64
   if (offset >  0x000000007fffffff)
      return XrdHdfsSys::Emsg(epname, error, EFBIG, "write", fname);
#endif

   return XrdHdfsSys::Emsg(epname,error,ENOTSUP, "Write mode not"
      " supported by HDFS.",fname);

}

/******************************************************************************/
/*                             W r i t e   A I O                              */
/******************************************************************************/
  
int XrdHdfsFile::Write(XrdSfsAio *aiop)
{

// Execute this request in a synchronous fashion
//
   aiop->Result = this->Write((const char *)aiop->sfsAio.aio_buf, aiop->sfsAio.aio_offset,
                              aiop->sfsAio.aio_nbytes);
   aiop->doneWrite();
   return 0;
}
  
/******************************************************************************/
/*                                F s t a t                                   */
/******************************************************************************/

int XrdHdfsFile::Fstat(struct stat     *buf)         // Out
/*
  Function: Return file status information

  Input:    buf         - The stat structiure to hold the results

  Output:   Returns XrdOssOK upon success and -errno upon failure.
*/
{
   static const char *epname = "stat";

   hdfsFileInfo * fileInfo = hdfsGetPathInfo(fs, fname);
// Execute the function
//
   if (fileInfo == NULL)
      return XrdHdfsSys::Emsg(epname, error, errno, "stat", fname);

   buf->st_mode = (fileInfo->mKind == kObjectKindDirectory) ? (S_IFDIR | 0777):\
      (S_IFREG | 0666);
   buf->st_nlink = (fileInfo->mKind == kObjectKindDirectory) ? 0 : 1;
   buf->st_uid = 1;
   buf->st_gid = 1;
   buf->st_size = (fileInfo->mKind == kObjectKindDirectory) ? 4096 : \
      fileInfo->mSize;
   buf->st_mtime    = fileInfo->mLastMod;
   buf->st_atime    = fileInfo->mLastMod;
   buf->st_ctime    = fileInfo->mLastMod;
   buf->st_dev      = 0;
   buf->st_ino      = 0;

   hdfsFreeFileInfo(fileInfo, 1);

// All went well
//
   return XrdOssOK;
}

/******************************************************************************/
/*         F i l e   S y s t e m   O b j e c t   I n t e r f a c e s          */
/******************************************************************************/
 
/******************************************************************************/
/*                                  I n i t                                   */
/******************************************************************************/

/*
  Function: Initialize HDFS system.

  Input:    None

  Output:   Returns zero upon success otherwise (-errno).
*/
int XrdHdfsSys::Init(XrdSysLogger *lp, const char *configfn)
{
   int NoGo;
   const char *tmp;

// Do the herald thing
//
   eDest = &OssEroute;
   eDest->logger(lp);
   eDest->Say("Copr. 2009, Brian Bockelman, Hdfs Version ");
   eDest->Emsg("Config", "Copr. 2009, Brian Bockelman, Hdfs Version ");

// Initialize the subsystems
//
   N2N_Lib=NULL;
   the_N2N=NULL;
   tmp = ((NoGo=Configure(configfn)) ? "failed." : "completed.");
   eDest->Say("------ HDFS storage system initialization ", tmp);
   eDest->Emsg("HDFS storage system initialization.", tmp);

// All done.
//
   return NoGo;
}
 
/******************************************************************************/
/*                            g e t V e r s i o n                             */
/******************************************************************************/

const char *XrdHdfsSys::getVersion() {return XrdVERSION;}

/******************************************************************************/
/*                                  S t a t                                   */
/******************************************************************************/

int XrdHdfsSys::Stat(const char              *path,        // In
                        struct stat       *buf,         // Out
                        int                )            // In
/*
  Function: Get info on 'path'.

  Input:    path        - Is the fully qualified name of the file to be tested.
            buf         - The stat structiure to hold the results
            error       - Error information object holding the details.
            client      - Authentication credentials, if any.
            info        - Opaque information, if any.

  Output:   Returns XrdOssOK upon success and -errno upon failure.
*/
{
   static const char *epname = "stat";
   int retc = XrdOssOK;
   char * fname;
   hdfsFileInfo * fileInfo = NULL;

   if (XrdHdfsSS.the_N2N) {
       char actual_path[XrdHdfsMAX_PATH_LEN+1];
       if ((XrdHdfsSS.the_N2N)->lfn2pfn(path, actual_path, sizeof(actual_path))) {
          (XrdHdfsSS.eDest)->Say("Cannot find a N2N mapping for ", path, "; using path directly.");
          fname = strdup(path);
       } else {
          fname = strdup(actual_path);
       }
   } else {
       fname = strdup(path);
   }

   hdfsFS fs = hdfsConnectAsUserNewInstance("default", 0, "nobody");
   if (fs == NULL) {
      retc = XrdHdfsSys::Emsg(epname, error, EIO, "stat", fname);
      goto cleanup;
   }

   fileInfo = hdfsGetPathInfo(fs, fname);

// Execute the function
//
   if (fileInfo == NULL) {
      retc = XrdHdfsSys::Emsg(epname, error, errno, "stat", fname);
      goto cleanup;
   }

   buf->st_mode = (fileInfo->mKind == kObjectKindDirectory) ? (S_IFDIR | 0777):\
      (S_IFREG | 0666);
   buf->st_nlink = (fileInfo->mKind == kObjectKindDirectory) ? 0 : 1;
   buf->st_uid = 1;
   buf->st_gid = 1;
   buf->st_size = (fileInfo->mKind == kObjectKindDirectory) ? 4096 : \
      fileInfo->mSize;
   buf->st_mtime    = fileInfo->mLastMod;
   buf->st_atime    = fileInfo->mLastMod;
   buf->st_ctime    = fileInfo->mLastMod;
   buf->st_dev      = 0;
   buf->st_ino      = 0;

   hdfsFreeFileInfo(fileInfo, 1);

// All went well
//
cleanup:
   if (fs)
      hdfsDisconnect(fs);
   if (fname)
      free(fname);
   return retc;
}


/******************************************************************************/
/*                                  E m s g                                   */
/******************************************************************************/

int XrdHdfsSys::Emsg(const char    *pfx,    // Message prefix value
                       XrdOucErrInfo &einfo,  // Place to put text & error code
                       int            ecode,  // The error code
                       const char    *op,     // Operation being performed
                       const char    *target) // The target (e.g., fname)
{
    char *etext, buffer[XrdOucEI::Max_Error_Len], unkbuff[64];

// Get the reason for the error
//
   if (ecode < 0) ecode = -ecode;
   if (!(etext = strerror(ecode)))
      {sprintf(unkbuff, "reason unknown (%d)", ecode); etext = unkbuff;}

// Format the error message
//
    snprintf(buffer,sizeof(buffer),"Unable to %s %s; %s", op, target, etext);

// Print it out if debugging is enabled
//
#ifndef NODEBUG
   OssEroute.Emsg(pfx, buffer);
#endif

// Place the error message in the error object and return
//
    einfo.setErrInfo(ecode, buffer);

    if (errno != 0)
       return (errno > 0) ? -errno : errno;
    return -1;
}
