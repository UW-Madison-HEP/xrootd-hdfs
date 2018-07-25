
/*
 * A checksum manager integrating with the Xrootd HDFS plugin.
 */

#include "XrdCks/XrdCks.hh"
#include "hdfs.h"
#include <openssl/evp.h>

#include <cstdio>
#include <vector>
#include <string>

#include "XrdOuc/XrdOucEnv.hh"

class XrdSysError;
class XrdOucEnv;

namespace XrdHdfs {

class ChecksumState
{
public:
    ChecksumState(unsigned digests);

    ~ChecksumState();

    void Update(const unsigned char *buff, size_t blen);

    void Finalize();

    std::string Get(unsigned digest);

private:
    const unsigned m_digests;
    uint32_t m_crc32;
    uint32_t m_cksum;
    uint32_t m_adler32;

    unsigned m_md5_length;
    size_t m_cur_chunk_bytes;
    off_t m_offset;

    EVP_MD_CTX *m_md5;
    EVP_MD_CTX *m_file_sha1;
    EVP_MD_CTX *m_chunk_sha1;

    unsigned char m_md5_value[EVP_MAX_MD_SIZE];
    std::string m_sha1_final; // Hex-encoded.
    std::string m_graft;

    struct CvmfsChunk
    {
        std::string m_sha1;
        off_t m_offset;
    };
    std::vector<CvmfsChunk> m_chunks;

};

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
        MD5     = 0x01,
        CKSUM   = 0x02,
        ADLER32 = 0x04,
        CVMFS   = 0x08,
        CRC32   = 0x10,
        ALL     = 0xff
    };

private:
    typedef std::pair<std::string, std::string> ChecksumValue;
    typedef std::vector<ChecksumValue> ChecksumValues;

    XrdSysError &m_log;
    XrdOucEnv m_client;

    std::string GetChecksumFilename(const char *pfn) const;
    int GetFileContents(const char *pfn, std::string &contents) const;
    int Parse(const std::string &chksum_contents, ChecksumValues &result);
    int SetMultiple(const char *pfn, const ChecksumValues &values) const;

    std::string m_default_digest;
};

}

