/******************************************************************************/
/*                                                                            */
/*                       X r d C m s T f c C o n f i g. c c                   */
/*                                                                            */
/*   Produced by Brian Bockelman for University of Nebraska                   */
/******************************************************************************/

const char *XrdHdfsConfigCVSID = "$Id$";

#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <memory.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/stat.h>

#include "XrdVersion.hh"
#include "XrdOuc/XrdOucEnv.hh"
#include "XrdSys/XrdSysError.hh"
#include "XrdSys/XrdSysHeaders.hh"
#include "XrdSys/XrdSysLogger.hh"
#include "XrdSys/XrdSysPlugin.hh"
#include "XrdSys/XrdSysPthread.hh"
#include "XrdSec/XrdSecInterface.hh"
#include "XrdHdfs/XrdHdfs.hh"

/******************************************************************************/
/*                               d e f i n e s                                */
/******************************************************************************/

#define TS_Xeq(x,m)    if (!strcmp(x,var)) return m(Config);

/******************************************************************************/
/*                             C o n f i g u r e                              */
/******************************************************************************/

int XrdHdfsSys::Configure(const char *cfn)
{
/*
  Function: Establish configuration at start up time.

  Input:    None.

  Output:   0 upon success or !0 otherwise.
*/
   int NoGo = 0;

   N2N_Lib = NULL;
   the_N2N = NULL;

   eDest->Emsg("Config", "Configuring HDFS.");

// Process the configuration file
//
   if ((NoGo = ConfigProc(cfn))) return NoGo;

// Allocate an Xroot proxy object (only one needed here)
//
   return 0;
}

/******************************************************************************/
/*                            C o n f i g P r o c                             */
/******************************************************************************/

int XrdHdfsSys::ConfigProc(const char *Cfn)
{
  char *var;
  int  cfgFD, retc, NoGo = 0;
  XrdOucEnv myEnv;
  XrdOucStream Config(eDest, getenv("XRDINSTANCE"), &myEnv, "=====> ");

// Make sure we have a config file
//
   if (!Cfn || !*Cfn)
      {eDest->Emsg("Config", "Configuration file not specified.");
       return 1;
      }

// Try to open the configuration file.
//
   if ( (cfgFD = open(Cfn, O_RDONLY, 0)) < 0)
      {eDest->Emsg("Config", errno, "open config file", Cfn);
       return 1;
      }
   ConfigFN = Cfn;

   Config.Attach(cfgFD);

// Now start reading records until eof.
//
   while((var = Config.GetMyFirstWord()))
        {if (!strncmp(var, "oss.", 4))
            if (ConfigXeq(var+4, Config)) {Config.Echo(); NoGo = 1;}
        }

// All done scanning the file, set dependent parameters.
//
   if (N2N_Lib) NoGo |= ConfigN2N();

// Now check if any errors occured during file i/o
//
   if ((retc = Config.LastError()))
      NoGo = eDest->Emsg("Config", retc, "read config file", Cfn);
   Config.Close();

// Return final return code
//
   return NoGo;
}

/******************************************************************************/
/*                             C o n f i g X e q                              */
/******************************************************************************/

int XrdHdfsSys::ConfigXeq(char *var, XrdOucStream &Config)
{

   // Process items. for either a local or a remote configuration
   //

   TS_Xeq("namelib",       xnml);

   // No match found, complain.
   //
   //eDest->Say("Config warning: ignoring unknown directive '",var,"'.");
   //Config.Echo();
   return 0;
}

/******************************************************************************/
/*                             C o n f i g N 2 N                              */
/******************************************************************************/

int XrdHdfsSys::ConfigN2N()
{
   XrdSysPlugin    *myLib;
   XrdOucName2Name *(*ep)(XrdOucgetName2NameArgs);

// Create a plugin object (we will throw this away without deletion because
// the library must stay open but we never want to reference it again).
//
   if (!(myLib = new XrdSysPlugin(eDest, N2N_Lib))) return 1;

// Now get the entry point of the object creator
//
   ep = (XrdOucName2Name *(*)(XrdOucgetName2NameArgs))(myLib->getPlugin("XrdOucgetName2Name"));
   if (!ep) return 1;


// Get the Object now
//
   the_N2N = ep(eDest, ConfigFN,
                (N2N_Parms ? N2N_Parms : ""),
                NULL, NULL);
   return the_N2N == 0;
}


/******************************************************************************/
/*                                  x n m l                                   */
/******************************************************************************/

/* Function: xnml

   Purpose:  To parse the directive: namelib <path> [<parms>]

             <path>    the path of the filesystem library to be used.
             <parms>   optional parms to be passed

  Output: 0 upon success or !0 upon failure.
*/

int XrdHdfsSys::xnml(XrdOucStream &Config)
{
    char *val, parms[1024];

// Get the path
//
   if (!(val = Config.GetWord()) || !val[0])
      {eDest->Emsg("Config", "namelib not specified"); return 1;}

// Record the path
//
   if (N2N_Lib) free(N2N_Lib);
   N2N_Lib = strdup(val);

// Record any parms
//
   if (!Config.GetRest(parms, sizeof(parms)))
      {eDest->Emsg("Config", "namelib parameters too long"); return 1;}
   if (N2N_Parms) free(N2N_Parms);
   N2N_Parms = (*parms ? strdup(parms) : 0);
   return 0;
}

