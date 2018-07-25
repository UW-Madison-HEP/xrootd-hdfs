#include <sstream>

#include "XrdVersion.hh"

#include "XrdHdfsChecksum.hh"

#include "XrdOss/XrdOss.hh"
#include "XrdSfs/XrdSfsInterface.hh"
#include "XrdSys/XrdSysError.hh"

XrdVERSIONINFO(XrdCksInit, XrdHdfsChecksum)

using namespace XrdHdfs;
class XrdSfsFileSystem;

typedef std::pair<std::string, std::string> ChecksumValue;
typedef std::vector<ChecksumValue> ChecksumValues;

extern XrdOss *g_hdfs_oss;

extern "C" {

XrdCks *XrdCksInit(XrdSysError *eDest,
                   const char *config_fn,
                   const char *params)
{
    XrdCks *cks = new ChecksumManager(*eDest);
    cks->Init(config_fn);
    return cks;
}

}


ChecksumManager::ChecksumManager(XrdSysError& log)
    : XrdCks(&log),
      m_log(log)
{
}


int
ChecksumManager::GetFileContents(const char *pfn, std::string &result) const
{
    if (!g_hdfs_oss) {return -ENOMEM;}

    std::string checksum_path = GetChecksumFilename(pfn);

    XrdOssDF *checksum_file = g_hdfs_oss->newFile("checksum_manager");
    if (!checksum_file) {return -ENOMEM;}

    int rc = checksum_file->Open(checksum_path.c_str(), SFS_O_RDONLY, 0, const_cast<XrdOucEnv &>(m_client));
    if (rc)
    {
        return rc;
    }

    std::vector<char> read_buffer;
    std::stringstream ss;
    const int buffer_size = 4096;
    read_buffer.resize(buffer_size);

    int retval = 0;
    off_t offset = 0;
    do
    {
        do
        {
            retval = checksum_file->Read(&read_buffer[0], offset, buffer_size-1);
        }
        while (retval == -EINTR);

        if (retval > 0)
        {
            read_buffer[retval] = '\0';
            ss << &read_buffer[0];
            offset += retval;
        }
    }
    while (retval > 0);
    checksum_file->Close();
    delete checksum_file;

    if (retval < 0)
    {
        return retval;
    }

    result = ss.str();

    return 0;
}


int
ChecksumManager::Parse(const std::string &checksum_contents, ChecksumValues &result)
{
    const char *ptr = checksum_contents.c_str();
    std::vector<char> checksum_buffer;
    checksum_buffer.resize(checksum_contents.size()+1);
    unsigned int length = 0;
    std::string cksum_value;

    while (sscanf(ptr, "%s%n", &checksum_buffer[0], &length))
    {
        if (strlen(&checksum_buffer[0]) < 2)
        {
            m_log.Emsg("Parse", "Too-short entry for checksum");
            return -EIO;
        }
        char *val = strchr(&checksum_buffer[0], ':');
        if (val == NULL)
        {
            m_log.Emsg("Parse", "Invalid format of checksum entry.");
            return -EIO;
        }
        *val = '\0';
        val++;
        if (*val == '\0')
        {
            m_log.Emsg("Parse", "Checksum value not specified");
            return -EIO;
        }
        ChecksumValue digest_and_val;
        digest_and_val.first = &checksum_buffer[0];
        digest_and_val.second = val;
        result.push_back(digest_and_val);

        ptr += length;
        if (*ptr == '\0')
        {
            break;
        }
        if (*ptr != '\n')
        {
            m_log.Emsg("Parse", "Invalid format of checksum entry (Not a newline)");
            return -EIO;
        }
        ptr += 1;
        if (*ptr == '\0')
        {
            break;
        }
    }
    return 0;
}


int
ChecksumManager::Init(const char * /*config_fn*/, const char *default_checksum)
{
    if (default_checksum)
    {
        m_default_digest = default_checksum;
    }
    return 0;
}

std::string
ChecksumManager::GetChecksumFilename(const char * pfn) const
{
    if (!pfn) {return "";}

    std::string filename = "/cksums/";
    filename += pfn;
    return filename;
}

int
ChecksumManager::Get(const char *pfn, XrdCksData &cks)
{
    std::string checksum_path = GetChecksumFilename(pfn);
    const char *requested_checksum = cks.Name ? cks.Name : m_default_digest.c_str();
    if (!strlen(requested_checksum))
    {
        requested_checksum = "adler32";
    }

    std::string checksum_contents;
    int rc = GetFileContents(pfn, checksum_contents);
    if (rc)
    {
        return rc;
    }
    ChecksumValues values;
    rc = Parse(checksum_contents, values);
    if (rc)
    {
        return rc;
    }

    std::string checksum_value;
    for (ChecksumValues::const_iterator iter = values.begin();
         iter != values.end();
         iter++)
    {
        if (!strcasecmp(iter->first.c_str(), requested_checksum))
        {
            checksum_value = iter->second;
        }
    }

    if (!checksum_value.size()) {
        Del(pfn, cks);
        return -ESRCH;
    }

    std::stringstream ss2;
    ss2 << "Got checksum (" << requested_checksum << ":" << checksum_value << ") for " << pfn;
    m_log.Emsg("Get", ss2.str().c_str());

    if (checksum_value.size() > cks.ValuSize)
    {
        m_log.Emsg("Get", "Recorded checksum is too long for file:", pfn);
        return -EDOM;
    }
    memcpy(cks.Value, checksum_value.c_str(), checksum_value.size());
    cks.Length = checksum_value.size();

    return 0;
}


int
ChecksumManager::Del(const char *pfn, XrdCksData &cks)
{
    return g_hdfs_oss->Unlink(pfn);
}


int
ChecksumManager::Config(const char *token, char *line)
{
    m_log.Emsg("Config", "Config variable passed", token, line);
    return 1;
}


char *
ChecksumManager::List(const char *pfn, char *buff, int blen, char separator)
{
    std::string checksum_contents;
    int rc = GetFileContents(pfn, checksum_contents);
    if (rc)
    {
        return NULL;
    }
    ChecksumValues values;
    rc = Parse(checksum_contents, values);
    if (rc)
    {
        return NULL;
    }

    std::stringstream ss;
    bool first_entry = true;
    for (ChecksumValues::const_iterator iter = values.begin();
         iter != values.end();
         iter++)
    {
        if (first_entry)
        {
            first_entry = false;
        }
        else
        {
            ss << separator;
        }
        ss << iter->first;
    }
    std::string result = ss.str();
    size_t mem_to_copy = (static_cast<unsigned>(blen) < result.size()) ? blen : result.size();
    memcpy(buff, result.c_str(), mem_to_copy);

    return buff;
}


const char *
ChecksumManager::Name(int seq_num)
{
    switch (seq_num)
    {
    case 0:
        return "md5";
    case 1:
        return "adler32";
    case 2:
        return "cksum";
    default:
        return NULL;
    }
}


int
ChecksumManager::Ver(const char *pfn, XrdCksData &cks)
{
    XrdCksData cks_on_disk;
    int rc = Get(pfn, cks_on_disk);
    if (rc)
    {
        rc = Calc(pfn, cks_on_disk, 1);
        if (rc)
        {
            return rc;
        }
    }

    return !memcmp(cks_on_disk.Value, cks.Value, cks.Length) ? 0 : 1;
}


int
ChecksumManager::Size(const char *name)
{
    if (!strcasecmp(name, "md5")) {return 16;}
    else if (!strcasecmp(name, "adler32")) {return 5;}
    else if (!strcasecmp(name, "cksum")) {return 5;}
    return -1;
}


int
ChecksumManager::Set(const char *pfn, XrdCksData &cks, int mtime)
{
    return 1;
}
