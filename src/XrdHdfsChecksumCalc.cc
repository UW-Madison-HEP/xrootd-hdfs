#include "XrdHdfsChecksum.hh"

using namespace XrdHdfs;


XrdCksCalc *
ChecksumManager::Object(const char * /*name*/)
{
    return NULL;
}

int
ChecksumManager::Calc(const char * /*pfn*/, XrdCksData & /*cks*/, int /* do_set */)
{
    // TODO: migrate the checksum calculation routines from gridftp-hdfs!
    return 1;
}
