/******************************************************************************/
/*                                                                            */
/*                       X r d H d f s . c c                                  */
/*                                                                            */
/* (c) 2004 by the Board of Trustees of the Leland Stanford, Jr., University  */
/*       All Rights Reserved. See XrdInfo.cc for complete License Terms       */
/*   Produced by Andrew Hanushevsky for Stanford University under contract    */
/*               DE-AC03-76-SFO0515 with the Deprtment of Energy              */
/******************************************************************************/

//          $Id: XrdHdfs.cc,v 1.20 2008/06/09 10:44:30 ganis Exp $

const char *XrdHdfsCVSID = "$Id: XrdHdfs.cc,v 1.20 2008/06/09 10:44:30 ganis Exp $";

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
#include "XrdSys/XrdSysPthread.hh"
#include "XrdSec/XrdSecInterface.hh"
#include "XrdHdfs/XrdHdfsAio.hh"
#include "XrdHdfs/XrdHdfs.hh"

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
/*                        G l o b a l   O b j e c t s                         */
/******************************************************************************/
  
XrdSysError    *XrdHdfs::eDest;

/******************************************************************************/
/*                           C o n s t r u c t o r                            */
/******************************************************************************/

XrdHdfs::XrdHdfs(XrdSysError *ep)
{
  eDest = ep;
}
  
/******************************************************************************/
/*                         G e t F i l e S y s t e m                          */
/******************************************************************************/
XrdSfsFileSystem *XrdSfsGetFileSystem(XrdSfsFileSystem *native_fs, 
                                       XrdSysLogger     *lp,
                                       const char *      config)
{
 static XrdSysError  Eroute(lp, "XrdHdfs");
 static XrdHdfs myFS(&Eroute);

 Eroute.Say("Copr.  2009 University of Nebraska-Lincoln"
               "hdfs plugin v 1.0a");

 return &myFS;
}

/******************************************************************************/
/*           D i r e c t o r y   O b j e c t   I n t e r f a c e s            */
/******************************************************************************/
/******************************************************************************/
/*                                  o p e n                                   */
/******************************************************************************/
  
int XrdHdfsDirectory::open(const char              *dir_path, // In
                                const XrdSecClientName  *client,   // In
                                const char              *info)     // In
/*
  Function: Open the directory `path' and prepare for reading.

  Input:    path      - The fully qualified name of the directory to open.
            cred      - Authentication credentials, if any.
            info      - Opaque information, if any.

  Output:   Returns SFS_OK upon success, otherwise SFS_ERROR.
*/
{
   static const char *epname = "opendir";

// Verify that this object is not already associated with an open directory
//
     if (dh) return
        XrdHdfs::Emsg(epname, error, EADDRINUSE, 
                             "open directory", dir_path);

// Set up values for this directory object
//
   fname = strdup(dir_path);
   dirPos = 0;

// Open the directory and get it's id
//
     if (!(dh = hdfsListDirectory(fs, dir_path, &numEntries))) return
        XrdHdfs::Emsg(epname,error,errno,"open directory",dir_path);

// All done
//
   return SFS_OK;
}

/******************************************************************************/
/*                             n e x t E n t r y                              */
/******************************************************************************/

const char *XrdHdfsDirectory::nextEntry()
/*
  Function: Read the next directory entry.

  Input:    None.

  Output:   Upon success, returns the contents of the next directory entry as
            a null terminated string. Returns a null pointer upon EOF or an
            error. To differentiate the two cases, getErrorInfo will return
            0 upon EOF and an actual error code (i.e., not 0) on error.
*/
{
    static const char *epname = "nextEntry";

// Lock the direcrtory and do any required tracing
//
   if (!dh) 
      {XrdHdfs::Emsg(epname,error,EBADF,"read directory",fname);
       return (const char *)0;
      }

// Check if we are at EOF (once there we stay there)
//
   if (dirPos == numEntries-1)
     return (const char *)0;

// Read the next directory entry
//
   hdfsFileInfo fileInfo = dh[dirPos];
   dirPos++;

// Return the actual entry
//
   return (const char *)(fileInfo.mName);
}

/******************************************************************************/
/*                                 c l o s e                                  */
/******************************************************************************/
  
int XrdHdfsDirectory::close()
/*
  Function: Close the directory object.

  Input:    cred       - Authentication credentials, if any.

  Output:   Returns SFS_OK upon success and SFS_ERROR upon failure.
*/
{

// Release the handle
//
    if (dh != NULL && numEntries > 0) hdfsFreeFileInfo(dh, numEntries);

// Do some clean-up
//
   if (fname) free(fname);
   dh = (hdfsFileInfo *)0; 
   numEntries = 0;
   dirPos = 0;
   return SFS_OK;
}

/******************************************************************************/
/*                F i l e   O b j e c t   I n t e r f a c e s                 */
/******************************************************************************/
/******************************************************************************/
/*                                  o p e n                                   */
/******************************************************************************/

int XrdHdfsFile::open(const char          *path,      // In
                           XrdSfsFileOpenMode   open_mode, // In
                           mode_t               Mode,      // In
                     const XrdSecClientName    *client,    // In
                     const char                *info)      // In
/*
  Function: Open the file `path' in the mode indicated by `open_mode'.  

  Input:    path      - The fully qualified name of the file to open.
            open_mode - One of the following flag values:
                        SFS_O_RDONLY - Open file for reading.
                        SFS_O_WRONLY - Open file for writing.
                        SFS_O_RDWR   - Open file for update
                        SFS_O_CREAT  - Create the file open in RDWR mode
                        SFS_O_TRUNC  - Trunc  the file open in RDWR mode
            Mode      - The Posix access mode bits to be assigned to the file.
                        These bits correspond to the standard Unix permission
                        bits (e.g., 744 == "rwxr--r--"). Mode may also conatin
                        SFS_O_MKPTH is the full path is to be created. The
                        agument is ignored unless open_mode = SFS_O_CREAT.
            client    - Authentication credentials, if any.
            info      - Opaque information to be used as seen fit.

  Output:   Returns OOSS_OK upon success, otherwise SFS_ERROR is returned.
*/
{
   static const char *epname = "open";
   const int AMode = S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH; // 775
   char *opname;
   int retc, open_flag = 0;

// Verify that this object is not already associated with an open file
//
   if (fh != NULL)
      return XrdHdfs::Emsg(epname,error,EADDRINUSE,"open file",path);
   fname = strdup(path);

// Set the actual open mode
//
   switch(open_mode & (SFS_O_RDONLY | SFS_O_WRONLY | SFS_O_RDWR))
   {
   case SFS_O_RDONLY: open_flag = O_RDONLY; break;
   case SFS_O_WRONLY: open_flag = O_WRONLY; break;
   case SFS_O_RDWR:   open_flag = O_RDWR;   break;
   default:           open_flag = O_RDONLY; break;
   }

   // HDFS does not support read-write mode.
   if (open_mode & SFS_O_RDWR) {
       return XrdHdfs::Emsg(epname,error,ENOTSUP, "Read-write mode not"
           " supported by HDFS.",path);
   }

// Prepare to create or open the file, as needed
//
   if (open_mode & SFS_O_CREAT)
      {
       opname = (char *)"create";
       if ((Mode & SFS_O_MKPTH) && (retc = XrdHdfs::Mkpath(path,AMode,info)))
          return XrdHdfs::Emsg(epname,error,retc,"create path for",path);
      } else if (open_mode & SFS_O_TRUNC)
                {open_flag  = O_TRUNC;
                 opname = (char *)"truncate";
                } else opname = (char *)"open";

// Open the file and make sure it is a file
//

   int err_code = 0;

   if ((fh = hdfsOpenFile(fs, path, open_flag, 0, 0, 0)) == NULL)
      {
       err_code = errno;
       if (errno == EEXIST)
          {
           hdfsFileInfo * fileInfo = hdfsGetPathInfo(fs, path);
           if (fileInfo != NULL) {
               if (fileInfo->mKind == kObjectKindDirectory) {
                   err_code = EISDIR;
               }
               hdfsFreeFileInfo(fileInfo, 1);
           }
          }
      }

// All done.
//
   if (err_code != 0)
       return XrdHdfs::Emsg(epname, error, err_code, opname, path);

   return SFS_OK;
}

/******************************************************************************/
/*                                 c l o s e                                  */
/******************************************************************************/

int XrdHdfsFile::close()
/*
  Function: Close the file object.

  Input:    None

  Output:   Returns SFS_OK upon success and SFS_ERROR upon failure.
*/
{
   static const char *epname = "close";

// Release the handle and return
//
    if (fh != NULL  && hdfsCloseFile(fs, fh) != 0)
       return XrdHdfs::Emsg(epname, error, errno, "close", fname);
    fh = NULL;
    if (fname) {free(fname); fname = 0;}
    return SFS_OK;
}

/******************************************************************************/
/*                                  f c t l                                   */
/******************************************************************************/

int      XrdHdfsFile::fctl(const int               cmd,
                                const char             *args,
                                      XrdOucErrInfo    &out_error)
{

// We don't support this
//
   out_error.setErrInfo(EEXIST, "fctl operation not supported");
   return SFS_ERROR;
}
  
/******************************************************************************/
/*                                  r e a d                                   */
/******************************************************************************/

XrdSfsXferSize XrdHdfsFile::read(XrdSfsFileOffset  offset,    // In
                                      char             *buff,      // Out
                                      XrdSfsXferSize    blen)      // In
/*
  Function: Read `blen' bytes at `offset' into 'buff' and return the actual
            number of bytes read.

  Input:    offset    - The absolute byte offset at which to start the read.
            buff      - Address of the buffer in which to place the data.
            blen      - The size of the buffer. This is the maximum number
                        of bytes that will be read from 'fd'.

  Output:   Returns the number of bytes read upon success and SFS_ERROR o/w.
*/
{
   static const char *epname = "read";
   XrdSfsXferSize nbytes;

// Make sure the offset is not too large
//
#if _FILE_OFFSET_BITS!=64
   if (offset >  0x000000007fffffff)
      return XrdHdfs::Emsg(epname, error, EFBIG, "read", fname);
#endif

// Read the actual number of bytes
//
   nbytes = hdfsPread(fs, fh, (off_t)offset, (void *)buff, (size_t)blen);

   if (nbytes  < 0)
      return XrdHdfs::Emsg(epname, error, errno, "read", fname);

// Return number of bytes read
//
   return nbytes;
}
  
/******************************************************************************/
/*                              r e a d   A I O                               */
/******************************************************************************/
  
int XrdHdfsFile::read(XrdSfsAio *aiop)
{

// Execute this request in a synchronous fashion
//
   aiop->Result = this->read((XrdSfsFileOffset)aiop->sfsAio.aio_offset,
                                       (char *)aiop->sfsAio.aio_buf,
                               (XrdSfsXferSize)aiop->sfsAio.aio_nbytes);
   aiop->doneRead();
   return 0;
}

/******************************************************************************/
/*                                 w r i t e                                  */
/******************************************************************************/

XrdSfsXferSize XrdHdfsFile::write(XrdSfsFileOffset   offset,    // In
                                       const char        *buff,      // In
                                       XrdSfsXferSize     blen)      // In
/*
  Function: Write `blen' bytes at `offset' from 'buff' and return the actual
            number of bytes written.

  Input:    offset    - The absolute byte offset at which to start the write.
            buff      - Address of the buffer from which to get the data.
            blen      - The size of the buffer. This is the maximum number
                        of bytes that will be written to 'fd'.

  Output:   Returns the number of bytes written upon success and SFS_ERROR o/w.

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
      return XrdHdfs::Emsg(epname, error, EFBIG, "write", fname);
#endif

   return XrdHdfs::Emsg(epname,error,ENOTSUP, "Write mode not"
      " supported by HDFS.",fname);

}

/******************************************************************************/
/*                             w r i t e   A I O                              */
/******************************************************************************/
  
int XrdHdfsFile::write(XrdSfsAio *aiop)
{

// Execute this request in a synchronous fashion
//
   aiop->Result = this->write((XrdSfsFileOffset)aiop->sfsAio.aio_offset,
                                        (char *)aiop->sfsAio.aio_buf,
                                (XrdSfsXferSize)aiop->sfsAio.aio_nbytes);
   aiop->doneWrite();
   return 0;
}
  
/******************************************************************************/
/*                                  s t a t                                   */
/******************************************************************************/

int XrdHdfsFile::stat(struct stat     *buf)         // Out
/*
  Function: Return file status information

  Input:    buf         - The stat structiure to hold the results

  Output:   Returns SFS_OK upon success and SFS_ERROR upon failure.
*/
{
   static const char *epname = "stat";

   hdfsFileInfo * fileInfo = hdfsGetPathInfo(fs, fname);
// Execute the function
//
   if (fileInfo == NULL)
      return XrdHdfs::Emsg(epname, error, errno, "state", fname);

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
   return SFS_OK;
}

/******************************************************************************/
/*                                  s y n c                                   */
/******************************************************************************/

int XrdHdfsFile::sync()
/*
  Function: Commit all unwritten bytes to physical media.

  Input:    None

  Output:   Returns SFS_OK upon success and SFS_ERROR upon failure.
*/
{
   static const char *epname = "sync";

   return XrdHdfs::Emsg(epname,error,ENOTSUP, "Write mode not"
      " supported by HDFS.",fname);

// All done
//
   return SFS_OK;
}

/******************************************************************************/
/*                              s y n c   A I O                               */
/******************************************************************************/
  
int XrdHdfsFile::sync(XrdSfsAio *aiop)
{

// Execute this request in a synchronous fashion
//
   aiop->Result = this->sync();
   aiop->doneWrite();
   return 0;
}

/******************************************************************************/
/*                              t r u n c a t e                               */
/******************************************************************************/

int XrdHdfsFile::truncate(XrdSfsFileOffset  flen)  // In
/*
  Function: Set the length of the file object to 'flen' bytes.

  Input:    flen      - The new size of the file.

  Output:   Returns SFS_OK upon success and SFS_ERROR upon failure.

  Notes:    If 'flen' is smaller than the current size of the file, the file
            is made smaller and the data past 'flen' is discarded. If 'flen'
            is larger than the current size of the file, a hole is created
            (i.e., the file is logically extended by filling the extra bytes 
            with zeroes).
*/
{
   static const char *epname = "trunc";

// Make sure the offset is not too larg
//
   if (sizeof(off_t) < sizeof(flen) && flen >  0x000000007fffffff)
      return XrdHdfs::Emsg(epname, error, EFBIG, "truncate", fname);

// Perform the function
//
   return XrdHdfs::Emsg(epname,error,ENOTSUP, "Truncate not"
      " supported by HDFS.",fname);

// All done
//
   return SFS_OK;
}

/******************************************************************************/
/*         F i l e   S y s t e m   O b j e c t   I n t e r f a c e s          */
/******************************************************************************/
/******************************************************************************/
/*                                 c h m o d                                  */
/******************************************************************************/

int XrdHdfs::chmod(const char             *path,    // In
                              XrdSfsMode        Mode,    // In
                              XrdOucErrInfo    &error,   // Out
                        const XrdSecClientName *client,  // In
                        const char             *info)    // In
/*
  Function: Change the mode on a file or directory.

  Input:    path      - Is the fully qualified name of the file to be removed.
            einfo     - Error information object to hold error details.
            client    - Authentication credentials, if any.
            info      - Opaque information, if any.

  Output:   Returns SFS_OK upon success and SFS_ERROR upon failure.
*/
{
   static const char *epname = "chmod";
   mode_t acc_mode = Mode & S_IAMB;

   const char * groups[1] = {client->grps};
   hdfsFS fs = hdfsConnectAsUser("default", 0, client->name, groups, 1);
      
// Perform the actual deletion
//
   if (hdfsChmod(fs, path, acc_mode) )
      return XrdHdfs::Emsg(epname,error,errno,"change mode on",path);

// All done
//
    return SFS_OK;
}
  
/******************************************************************************/
/*                                e x i s t s                                 */
/******************************************************************************/

int XrdHdfs::exists(const char                *path,        // In
                               XrdSfsFileExistence &file_exists, // Out
                               XrdOucErrInfo       &error,       // Out
                         const XrdSecClientName    *client,      // In
                         const char                *info)        // In
/*
  Function: Determine if file 'path' actually exists.

  Input:    path        - Is the fully qualified name of the file to be tested.
            file_exists - Is the address of the variable to hold the status of
                          'path' when success is returned. The values may be:
                          XrdSfsFileExistsIsDirectory - file not found but path is valid.
                          XrdSfsFileExistsIsFile      - file found.
                          XrdSfsFileExistsIsNo        - neither file nor directory.
            einfo       - Error information object holding the details.
            client      - Authentication credentials, if any.
            info        - Opaque information, if any.

  Output:   Returns SFS_OK upon success and SFS_ERROR upon failure.

  Notes:    When failure occurs, 'file_exists' is not modified.
*/
{
   static const char *epname = "exists";

// Now try to find the file or directory
//

   const char * groups[1] = {client->grps};
   hdfsFS fs = hdfsConnectAsUser("default", 0, client->name, groups, 1);

   hdfsFileInfo * fileInfo = hdfsGetPathInfo(fs, path);

   if (fileInfo == NULL)
       return XrdHdfs::Emsg(epname, error, errno, "locate", path);

   if (fileInfo->mKind == kObjectKindDirectory)
      file_exists = XrdSfsFileExistIsDirectory;
   else if (fileInfo->mKind == kObjectKindFile)
      file_exists = XrdSfsFileExistIsFile;
   else
      file_exists = XrdSfsFileExistNo;

   hdfsFreeFileInfo(fileInfo, 1);

   return SFS_OK;
}

/******************************************************************************/
/*                                 f s c t l                                  */
/******************************************************************************/

int XrdHdfs::fsctl(const int               cmd,
                        const char             *args,
                              XrdOucErrInfo    &out_error,
                        const XrdSecClientName *client)
{
    out_error.setErrInfo(ENOTSUP, "Operation not supported.");
    return SFS_ERROR;
}
  
/******************************************************************************/
/*                            g e t V e r s i o n                             */
/******************************************************************************/

const char *XrdHdfs::getVersion() {return XrdVERSION;}

/******************************************************************************/
/*                                 m k d i r                                  */
/******************************************************************************/

int XrdHdfs::mkdir(const char             *path,    // In
                              XrdSfsMode        Mode,    // In
                              XrdOucErrInfo    &error,   // Out
                        const XrdSecClientName *client,  // In
                        const char             *info)    // In
/*
  Function: Create a directory entry.

  Input:    path      - Is the fully qualified name of the file to be removed.
            Mode      - Is the POSIX mode setting for the directory. If the
                        mode contains SFS_O_MKPTH, the full path is created.
            einfo     - Error information object to hold error details.
            client    - Authentication credentials, if any.
            info      - Opaque information, if any.

  Output:   Returns SFS_OK upon success and SFS_ERROR upon failure.
*/
{
   static const char *epname = "mkdir";
   mode_t acc_mode = Mode & S_IAMB;

   const char * groups[1] = {client->grps};
   hdfsFS fs = hdfsConnectAsUser("default", 0, client->name, groups, 1);

// Create the path if it does not already exist
//
   if (Mode & SFS_O_MKPTH) Mkpath(path, acc_mode, info);

// Perform the actual deletion
//
   if (hdfsCreateDirectory(fs, path) == -1)
      return XrdHdfs::Emsg(epname,error,errno,"create directory",path);

// All done
//
    return SFS_OK;
}

/******************************************************************************/
/*                                M k p a t h                                 */
/******************************************************************************/
/*
  Function: Create a directory path

  Input:    path        - Is the fully qualified name of the new path.
            mode        - The new mode that each new directory is to have.
            info        - Opaque information, of any.

  Output:   Returns 0 upon success and -errno upon failure.
*/

int XrdHdfs::Mkpath(const char *path, mode_t mode, const char *info)
{

   return -ENOTSUP;

}

/******************************************************************************/
/*                                   r e m                                    */
/******************************************************************************/
  
int XrdHdfs::rem(const char             *path,    // In
                            XrdOucErrInfo    &error,   // Out
                      const XrdSecClientName *client,  // In
                      const char             *info)    // In
/*
  Function: Delete a file from the namespace.

  Input:    path      - Is the fully qualified name of the file to be removed.
            einfo     - Error information object to hold error details.
            client    - Authentication credentials, if any.
            info      - Opaque information, if any.

  Output:   Returns SFS_OK upon success and SFS_ERROR upon failure.
*/
{
   static const char *epname = "rem";
   const char * groups[1] = {client->grps};
   hdfsFS fs = hdfsConnectAsUser("default", 0, client->name, groups, 1);

// Perform the actual deletion
//
    if (hdfsDelete(fs, path) == -1)
       return XrdHdfs::Emsg(epname, error, errno, "remove", path);

// All done
//
    return SFS_OK;
}

/******************************************************************************/
/*                                r e m d i r                                 */
/******************************************************************************/

int XrdHdfs::remdir(const char             *path,    // In
                               XrdOucErrInfo    &error,   // Out
                         const XrdSecClientName *client,  // In
                         const char             *info)    // In
/*
  Function: Delete a directory from the namespace.

  Input:    path      - Is the fully qualified name of the dir to be removed.
            einfo     - Error information object to hold error details.
            client    - Authentication credentials, if any.
            info      - Opaque information, if any.

  Output:   Returns SFS_OK upon success and SFS_ERROR upon failure.
*/
{
   static const char *epname = "remdir";
   const char * groups[1] = {client->grps};
   hdfsFS fs = hdfsConnectAsUser("default", 0, client->name, groups, 1);

// Perform the actual deletion
//
    if (hdfsDelete(fs, path) == -1)
       return XrdHdfs::Emsg(epname, error, errno, "remove", path);

// All done
//
    return SFS_OK;
}

/******************************************************************************/
/*                                r e n a m e                                 */
/******************************************************************************/

int XrdHdfs::rename(const char             *old_name,  // In
                         const char             *new_name,  // In
                               XrdOucErrInfo    &error,     //Out
                         const XrdSecClientName *client,    // In
                         const char             *infoO,     // In
                         const char             *infoN)     // In
/*
  Function: Renames a file/directory with name 'old_name' to 'new_name'.

  Input:    old_name  - Is the fully qualified name of the file to be renamed.
            new_name  - Is the fully qualified name that the file is to have.
            error     - Error information structure, if an error occurs.
            client    - Authentication credentials, if any.
            info      - old_name opaque information, if any.
            info      - new_name opaque information, if any.

  Output:   Returns SFS_OK upon success and SFS_ERROR upon failure.
*/
{
   static const char *epname = "rename";
   const char * groups[1] = {client->grps};
   hdfsFS fs = hdfsConnectAsUser("default", 0, client->name, groups, 1);

// Perform actual rename operation
//
   if (hdfsRename(fs, old_name, new_name) == -1)
      return XrdHdfs::Emsg(epname, error, errno, "rename", old_name);

// All done
//
   return SFS_OK;
}
  
/******************************************************************************/
/*                                  s t a t                                   */
/******************************************************************************/

int XrdHdfs::stat(const char              *path,        // In
                             struct stat       *buf,         // Out
                             XrdOucErrInfo     &error,       // Out
                       const XrdSecClientName  *client,      // In
                       const char              *info)        // In
/*
  Function: Get info on 'path'.

  Input:    path        - Is the fully qualified name of the file to be tested.
            buf         - The stat structiure to hold the results
            error       - Error information object holding the details.
            client      - Authentication credentials, if any.
            info        - Opaque information, if any.

  Output:   Returns SFS_OK upon success and SFS_ERROR upon failure.
*/
{
   static const char *epname = "stat";
   const char * groups[1] = {client->grps};
   hdfsFS fs = hdfsConnectAsUser("default", 0, client->name, groups, 1);

   hdfsFileInfo * fileInfo = hdfsGetPathInfo(fs, path);

// Execute the function
//
   if (fileInfo == NULL)
      return XrdHdfs::Emsg(epname, error, errno, "state", path);

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
   return SFS_OK;
}

/******************************************************************************/
/*                              t r u n c a t e                               */
/******************************************************************************/
  
int XrdHdfs::truncate(const char             *path,    // In
                                 XrdSfsFileOffset  flen,    // In
                                 XrdOucErrInfo    &error,   // Out
                           const XrdSecClientName *client,  // In
                           const char             *info)    // In
/*
  Function: Set the length of the file object to 'flen' bytes.

  Input:    path      - The path to the file.
            flen      - The new size of the file.
            einfo     - Error information object to hold error details.
            client    - Authentication credentials, if any.
            info      - Opaque information, if any.

  Output:   Returns SFS_OK upon success and SFS_ERROR upon failure.

  Notes:    If 'flen' is smaller than the current size of the file, the file
            is made smaller and the data past 'flen' is discarded. If 'flen'
            is larger than the current size of the file, a hole is created
            (i.e., the file is logically extended by filling the extra bytes 
            with zeroes).
*/
{
   static const char *epname = "trunc";

   return XrdHdfs::Emsg(epname,error,ENOTSUP, "Truncate not"
      " supported by HDFS.",path);
}

/******************************************************************************/
/*                                  E m s g                                   */
/******************************************************************************/

int XrdHdfs::Emsg(const char    *pfx,    // Message prefix value
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
   eDest->Emsg(pfx, buffer);
#endif

// Place the error message in the error object and return
//
    einfo.setErrInfo(ecode, buffer);

    return SFS_ERROR;
}
