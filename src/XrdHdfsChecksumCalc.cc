#include "XrdHdfsChecksum.hh"

#include <sstream>

#include <arpa/inet.h>
#include <zlib.h>
#include <openssl/evp.h>

#include "XrdOss/XrdOss.hh"
#include "XrdSfs/XrdSfsInterface.hh"

using namespace XrdHdfs;

extern XrdOss *g_hdfs_oss;


#define CVMFS_CHUNK_SIZE (24*1024*1024)

// CRC32 table from the published POSIX standard
static uint32_t const g_crctab[256] =
{
  0x00000000,
  0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,
  0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f, 0x2f8ad6d6,
  0x2b4bcb61, 0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
  0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9, 0x5f15adac,
  0x5bd4b01b, 0x569796c2, 0x52568b75, 0x6a1936c8, 0x6ed82b7f,
  0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3, 0x709f7b7a,
  0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
  0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58,
  0xbaea46ef, 0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033,
  0xa4ad16ea, 0xa06c0b5d, 0xd4326d90, 0xd0f37027, 0xddb056fe,
  0xd9714b49, 0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
  0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1, 0xe13ef6f4,
  0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0,
  0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5,
  0x2ac12072, 0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16,
  0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca, 0x7897ab07,
  0x7c56b6b0, 0x71159069, 0x75d48dde, 0x6b93dddb, 0x6f52c06c,
  0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1,
  0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
  0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b,
  0xbb60adfc, 0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698,
  0x832f1041, 0x87ee0df6, 0x99a95df3, 0x9d684044, 0x902b669d,
  0x94ea7b2a, 0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e,
  0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2, 0xc6bcf05f,
  0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,
  0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80,
  0x644fc637, 0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
  0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f, 0x5c007b8a,
  0x58c1663d, 0x558240e4, 0x51435d53, 0x251d3b9e, 0x21dc2629,
  0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5, 0x3f9b762c,
  0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
  0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e,
  0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65,
  0xeba91bbc, 0xef68060b, 0xd727bbb6, 0xd3e6a601, 0xdea580d8,
  0xda649d6f, 0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
  0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7, 0xae3afba2,
  0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71,
  0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74,
  0x857130c3, 0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640,
  0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c, 0x7b827d21,
  0x7f436096, 0x7200464f, 0x76c15bf8, 0x68860bfd, 0x6c47164a,
  0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e, 0x18197087,
  0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
  0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d,
  0x2056cd3a, 0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce,
  0xcc2b1d17, 0xc8ea00a0, 0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb,
  0xdbee767c, 0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18,
  0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4, 0x89b8fd09,
  0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662,
  0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf,
  0xa2f33668, 0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
};


static std::string
human_readable_evp(unsigned char *evp, size_t length)
{
    unsigned int idx;
    std::string result; result.reserve(length*2);
    for (idx = 0; idx < length; idx++)
    {
        char encoded[2];
        sprintf(encoded, "%02x", evp[idx]);
        result += encoded;
    }
    return result;
}


ChecksumState::ChecksumState(unsigned digests)
    : m_digests(digests),
      m_cksum(0),
      m_adler32(adler32(0, NULL, 0)),
      m_md5_length(0),
      m_cur_chunk_bytes(0),
      m_offset(0),
      m_md5(NULL),
      m_file_sha1(NULL),
      m_chunk_sha1(NULL)
{
    if (digests & ChecksumManager::MD5)
    {
        m_md5 = EVP_MD_CTX_create();
        EVP_DigestInit_ex(m_md5, EVP_md5(), NULL);
    }
    if (digests & ChecksumManager::CVMFS)
    {
        m_file_sha1 = EVP_MD_CTX_create();
        EVP_DigestInit_ex(m_file_sha1, EVP_sha1(), NULL);
        m_chunk_sha1 = EVP_MD_CTX_create();
        EVP_DigestInit_ex(m_chunk_sha1, EVP_sha1(), NULL);
    }
}


ChecksumState::~ChecksumState()
{
    if (m_md5)
    {
        EVP_MD_CTX_destroy(m_md5);
    }
    if (m_file_sha1)
    {
        EVP_MD_CTX_destroy(m_file_sha1);
    }
    if (m_chunk_sha1)
    {
        EVP_MD_CTX_destroy(m_chunk_sha1);
    }
}


std::string
ChecksumState::Get(unsigned digest)
{
    if ((digest & ChecksumManager::CKSUM) && (m_digests & ChecksumManager::CKSUM))
    {
        uint32_t cksum_no = htonl(m_cksum);
        return human_readable_evp(reinterpret_cast<unsigned char *>(&cksum_no), sizeof(cksum_no));
    }
    if ((digest & ChecksumManager::ADLER32) && (m_digests & ChecksumManager::ADLER32))
    {
        uint32_t adler32_no = htonl(m_adler32);
        return human_readable_evp(reinterpret_cast<unsigned char *>(&adler32_no), sizeof(adler32_no));
    }
    if ((digest & ChecksumManager::MD5) && (m_digests & ChecksumManager::MD5))
    {
        return human_readable_evp(m_md5_value, m_md5_length);
    }
    if ((digest & ChecksumManager::CVMFS) && (m_digests & ChecksumManager::CVMFS))
    {
        return m_graft;
    }

    return "";
}

void
ChecksumState::Update(const unsigned char *buffer, size_t bsize)
{
    m_offset += bsize;
    if (m_digests & ChecksumManager::ADLER32)
    {
        m_adler32 = adler32(m_adler32, buffer, bsize);
    }
    if (m_digests & ChecksumManager::CKSUM)
    {
        size_t bytes_remaining = bsize;
        const unsigned char *current_buffer = buffer;
        uint32_t crc = m_cksum;
        while (bytes_remaining--) {
            crc = (crc << 8) ^ g_crctab[((crc >> 24) ^ *current_buffer++) & 0xFF];
        }
        m_cksum = crc;
    }
    if (m_digests & ChecksumManager::CRC32)
    {
        m_crc32 = crc32(m_crc32, buffer, bsize);
    }
    if (m_digests & ChecksumManager::MD5)
    {
        EVP_DigestUpdate(m_md5, buffer, bsize);
    }
    if (m_digests & ChecksumManager::CVMFS)
    {
        EVP_DigestUpdate(m_file_sha1, buffer, bsize);
        off_t total_bytes = m_cur_chunk_bytes + bsize;
        size_t buffer_offset = 0;
        while (total_bytes >= CVMFS_CHUNK_SIZE) {  // There are at least CVMFS_CHUNK_SIZE bytes to write!
            size_t new_bytes = CVMFS_CHUNK_SIZE - m_cur_chunk_bytes;
            EVP_DigestUpdate(m_chunk_sha1, buffer + buffer_offset, new_bytes);
            buffer_offset += new_bytes;
            bsize-= new_bytes;

            // Create a new chunk.
            unsigned char sha1_value[EVP_MAX_MD_SIZE];
            unsigned int sha1_len;
            EVP_DigestFinal_ex(m_chunk_sha1, sha1_value, &sha1_len);
            EVP_DigestInit_ex(m_chunk_sha1, EVP_sha1(), NULL);
            CvmfsChunk new_chunk;
            new_chunk.m_offset = (m_chunks.size() == 0) ? 0 : (m_chunks.back().m_offset + CVMFS_CHUNK_SIZE);
            new_chunk.m_sha1 = human_readable_evp(sha1_value, sha1_len);
            m_chunks.push_back(new_chunk);

            m_cur_chunk_bytes = 0;
            total_bytes -= CVMFS_CHUNK_SIZE;
        }
        EVP_DigestUpdate(m_chunk_sha1, buffer + buffer_offset, bsize);
        m_cur_chunk_bytes += bsize;
    }
}


void
ChecksumState::Finalize()
{
    if (m_digests & ChecksumManager::MD5)
    {
        EVP_DigestFinal_ex(m_md5, m_md5_value, &m_md5_length);
        EVP_MD_CTX_destroy(m_md5);
        m_md5 = NULL;
    }
    if (m_digests & ChecksumManager::CVMFS)
    {
        unsigned char sha1_value[EVP_MAX_MD_SIZE];
        unsigned int sha1_len;
        EVP_DigestFinal_ex(m_file_sha1, sha1_value, &sha1_len);
        EVP_MD_CTX_destroy(m_file_sha1);
        m_file_sha1 = NULL;
        m_sha1_final = human_readable_evp(sha1_value, sha1_len);

        off_t chunk_offset = m_offset - m_cur_chunk_bytes;
        if (m_cur_chunk_bytes && chunk_offset)
        {
            CvmfsChunk new_chunk;
            new_chunk.m_offset = chunk_offset;
            EVP_DigestFinal_ex(m_chunk_sha1, sha1_value, &sha1_len);
            new_chunk.m_sha1 = human_readable_evp(sha1_value, sha1_len);
        }
        EVP_MD_CTX_destroy(m_chunk_sha1);
        m_chunk_sha1 = NULL;

        std::stringstream ss;
        ss << "size=" << m_offset << ";checksum=" << m_sha1_final;
        if (m_chunks.size() < 2)
        {
            ss << ";chunk_offsets=0;chunk_checksums=" << m_sha1_final;
        }
        else
        {
            ss << ";chunk_offsets=0";
            for (unsigned idx = 1; idx < m_chunks.size(); idx++)
            {
                ss << "," << m_chunks[idx].m_offset;
            }
            ss << ";chunk_checksums=" << m_chunks[0].m_sha1;
            for (unsigned idx = 1; idx < m_chunks.size(); idx++)
            {
                ss << "," << m_chunks[idx].m_sha1;
            }
        }
        m_graft = ss.str();
    }
}


/*
 * Note - it is not apparent this is ever used, hence it is
 * just a stub in this implementation.
 */
XrdCksCalc *
ChecksumManager::Object(const char * /*name*/)
{
    return NULL;
}

int
ChecksumManager::Calc(const char *pfn, XrdCksData &cks, int do_set)
{
    int digests = 0;
    int return_digest = 0;
    if (do_set)
    {
        digests = ChecksumManager::ALL;
    }
    if (!strncasecmp(cks.Name, "md5", cks.NameSize))
    {
        return_digest = ChecksumManager::MD5;
    }
    else if (!strncasecmp(cks.Name, "cksum", cks.NameSize))
    {
        return_digest = ChecksumManager::CKSUM;
    }
    else if (!strncasecmp(cks.Name, "crc32", cks.NameSize))
    {
        return_digest = ChecksumManager::CRC32;
    }
    else if (!strncasecmp(cks.Name, "adler32", cks.NameSize))
    {
        return_digest = ChecksumManager::ADLER32;
    }
    else
    {
        return -ENOTSUP;
    }
    digests |= return_digest;

    if (!g_hdfs_oss) {return -ENOMEM;}

    XrdOssDF *fh = g_hdfs_oss->newFile("checksum_calc");
    if (!fh) {return -ENOMEM;}
    int rc = fh->Open(pfn, SFS_O_RDONLY, 0, m_client);
    if (rc)
    {
        return rc;
    }

    ChecksumState state(digests);

    const static int buffer_size = 256*1024;
    std::vector<unsigned char> read_buffer;
    read_buffer.reserve(buffer_size);

    int retval = 0;
    off_t offset = 0;
    do
    {
        do
        {
            retval = fh->Read(&read_buffer[0], offset, buffer_size);
        }
        while (retval == -EINTR);

        if (retval > 0)
        {
            state.Update(&read_buffer[0], retval);
            offset += retval;
        }
    }
    while (retval > 0);
    fh->Close();
    delete fh;

    if (retval < 0)
    {
        return -EIO;
    }

    state.Finalize();

    ChecksumValues values;
    if (digests & ChecksumManager::CKSUM)
    {
        ChecksumValue value;
        value.first = "CKSUM";
        value.second = state.Get(ChecksumManager::CKSUM);
        if (value.second.size()) {values.push_back(value);}
        if (return_digest == ChecksumManager::CKSUM)
        {
            if (!value.second.size()) return -EIO;
            cks.Set(value.second.c_str(), value.second.size());
        }
    }
    if (digests & ChecksumManager::ADLER32)
    {
        ChecksumValue value;
        value.first = "ADLER32";
        value.second = state.Get(ChecksumManager::ADLER32);
        if (value.second.size()) {values.push_back(value);}
        if (return_digest == ChecksumManager::ADLER32)
        {
            if (!value.second.size()) return -EIO;
            cks.Set(value.second.c_str(), value.second.size());
        }
    }
    if (digests & ChecksumManager::MD5)
    {
        ChecksumValue value;
        value.first = "MD5";
        value.second = state.Get(ChecksumManager::MD5);
        if (value.second.size()) {values.push_back(value);}
        if (return_digest == ChecksumManager::MD5)
        {
            if (!value.second.size()) return -EIO;
            cks.Set(value.second.c_str(), value.second.size());
        }
    }
    if (digests & ChecksumManager::CVMFS)
    {
        ChecksumValue value;
        value.first = "CVMFS";
        value.second = state.Get(ChecksumManager::CVMFS);
        if (value.second.size()) {values.push_back(value);}
    }

    SetMultiple(pfn, values); // Ignore return value - this is simply advisory.
    return 0;
}
