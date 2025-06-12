#include "request.hh"

#include "zlib.h"

#include <fstream>
#include <string>
using namespace std::string_literals;

#define CHUNK 16384

static Destination* build_destination(Options const& opt)
{
    auto ds = new Destination;
    ds->set_microcontroller(Destination_Microcontroller_AUTO);

    if (!opt.core.empty()) {
        if (opt.core == "auto")
            ds->set_microcontroller(Destination_Microcontroller_AUTO);
        else if (opt.core == "pico1")
            ds->set_microcontroller(Destination_Microcontroller_PICO_1);
        else if (opt.core == "pico2")
            ds->set_microcontroller(Destination_Microcontroller_PICO_2);
        else if (opt.core == "avr")
            ds->set_microcontroller(Destination_Microcontroller_AVR);
        else
            throw std::runtime_error("Invalid core.");
    }

    if (!opt.part.empty())
        ds->set_part(opt.part);
    if (!opt.baud)
        ds->set_baud(*opt.baud);

    return ds;
}

static std::string build_payload(std::string const& file)
{
    std::string payload;
    z_stream strm {};

    int ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
    if (ret != Z_OK)
        throw std::runtime_error("Compression error");

    std::ifstream f(file, std::ios::binary);
    if (!f.is_open())
        throw std::runtime_error("Can't read file '"s + file + "'");

    uint8_t in[CHUNK], out[CHUNK];

    int flush;
    do {
        f.read((char *) in, CHUNK);
        flush = f.eof() ? Z_FINISH : Z_NO_FLUSH;

        strm.avail_in = f.gcount();
        strm.next_in = in;

        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);
            unsigned have = CHUNK - strm.avail_out;

            size_t sz = payload.length();
            payload.resize(sz + have);
            memcpy(&payload[sz], out, have);
        } while (strm.avail_out == 0);

    } while (flush != Z_FINISH);

    deflateEnd(&strm);
    return payload;
}

Request build_request(Options const& opt)
{
    Request request;

    if (opt.command == "upload") {
        auto upload = new Request_FirmwareUpload;
        upload->set_allocated_destination(build_destination(opt));
        upload->set_verify(opt.verify);
        upload->set_verbose(opt.verbose);
        upload->set_payload_compressed(true);
        upload->set_payload(build_payload(opt.file));
        request.set_allocated_firmware_upload(upload);
    } else if (opt.command == "test") {
        request.set_allocated_test_connection(build_destination(opt));
    } else if (opt.command == "fuse") {
        auto fuse = new Request_AvrFuseProgramming;
        fuse->set_allocated_destination(build_destination(opt));
        fuse->set_low(*opt.fuse_low);
        fuse->set_high(*opt.fuse_high);
        if (opt.fuse_extended) {
            fuse->set_has_extended(true);
            fuse->set_extended(*opt.fuse_extended);
        }
        request.set_allocated_fuse_programming(fuse);
    } else if (opt.command == "reset") {
        auto reset = new Request_Reset;
        if (opt.wtime) reset->set_time_ms(*opt.wtime);
        request.set_allocated_reset(reset);
    } else if (opt.command == "spi") {
        throw std::runtime_error("Not implemented");  // TODO
    } else if (opt.command == "i2c") {
        throw std::runtime_error("Not implemented");  // TODO
    } else {
        throw std::runtime_error("Invalid command '"s + opt.command + "'.");
    }

    return request;
}
