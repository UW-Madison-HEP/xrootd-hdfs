
/*
 * A checksum manager integrating with the Xrootd HDFS plugin.
 */

#include "XrdCks/XrdCks.hh"
#include "hdfs.h"

#include <cstdio>
#include <vector>
#include <string>

#include "XrdOuc/XrdOucEnv.hh"

class XrdSysError;
class XrdOucEnv;

namespace XrdHdfs {

class ChecksumManager : public XrdCks
{
public:
    ChecksumManager(XrdSysError &);

    virtual int Calc(const char *pfn, XrdCksData &cks, int do_set=1) override;

    virtual int Del(const char *pfn, XrdCksData &cks) override;

    virtual int Get(const char *pfn, XrdCksData &cks) override;

    virtual int Config(const char *token, char *line) override;

    virtual int Init(const char *config_fn, const char *default_checksum = NULL) override;

    virtual char *List(const char *pfn, char *buff, int blen, char seperator=' ') override;

    virtual const char *Name(int seq_num) override;

    virtual XrdCksCalc *Object(const char *name) override;

    virtual int Size(const char *name=NULL);

    virtual int Set(const char *pfn, XrdCksData &cks, int mtime=0);

    virtual int Ver(const char *pfn, XrdCksData &cks);

    virtual ~ChecksumManager() {}

    enum ChecksumTypes {
        MD5,
        CKSUM,
        ADLER32
    };

private:
    typedef std::pair<std::string, std::string> ChecksumValue;
    typedef std::vector<ChecksumValue> ChecksumValues;

    XrdSysError &m_log;
    XrdOucEnv m_client;

    std::string GetChecksumFilename(const char *pfn) const;
    int GetFileContents(const char * pfn, std::string &contents) const;
    int Parse(const std::string &chksum_contents, ChecksumValues &result);

    std::string m_default_digest;
};

}

