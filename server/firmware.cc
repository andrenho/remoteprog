#include "firmware.hh"

#include <fstream>
#include <filesystem>

#include "zlib.h"

#define CHUNK 16384

namespace firmware {

std::string temp_filename()
{
    return std::string(getenv("TMPDIR")) + "/remoteprog.bin";
}

std::string save_file(std::string const& payload)
{
    auto file = temp_filename();
    std::ofstream f(file, std::ios::binary);
    if (!f.is_open())
        throw std::runtime_error("Could not open temporary file for upload");
    f.write(payload.c_str(), payload.length());
    f.close();

    return file;
}

std::string uncompress_file(std::string const& payload)
{
    auto file = temp_filename();
    std::ofstream f(file, std::ios::binary);
    if (!f.is_open())
        throw std::runtime_error("Could not open temporary file for upload");

    z_stream strm;
    uint8_t out[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    int ret = inflateInit(&strm);
    if (ret != Z_OK)
        throw std::runtime_error("Could not decompress file");

    /* decompress until deflate stream ends or end of file */
    size_t ptr = 0;
    do {
        strm.avail_in = std::min(payload.length() - ptr, (unsigned long) CHUNK);
        if (strm.avail_in == 0)
            break;
        strm.next_in = (uint8_t *) &payload[ptr];
        ptr += strm.avail_in;

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            switch (ret) {
                case Z_NEED_DICT:
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    (void)inflateEnd(&strm);
                    throw std::runtime_error("Error decompressing payload file");
            }
            size_t have = CHUNK - strm.avail_out;
            f.write((char *) out, have);
        } while (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);

    /* clean up and return */
    inflateEnd(&strm);
    f.close();

    return file;
}

Response_Result* upload(Request_FirmwareUpload const& req)
{
    auto result = new Response_Result;

    std::string filename;
    if (req.payload_compressed())
        filename = uncompress_file(req.payload());
    else
        filename = save_file(req.payload());

    // TODO - call programmer

    // TODO - delete temp file

    result->set_result_code(Response_ResultCode_SUCCESS);
    return result;
}

}
