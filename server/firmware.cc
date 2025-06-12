#include "firmware.hh"

#include <fstream>
#include <filesystem>
#include <vector>
#include <string>

#include "runner.hh"
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

static Destination find_destination(Destination const& destination)
{
    if (destination.microcontroller() == Destination_Microcontroller_AUTO)
        throw std::runtime_error("Destination AUTO not implemented yet.");  // TODO

    if (destination.microcontroller() == Destination_Microcontroller_AVR && destination.part().empty())
        throw std::runtime_error("AVR require a part (ex. atmega328).");

    return destination;
}

static void upload_payload(int fd, Destination const& destination, std::string const& payload_filename, bool verify, bool debug_mode)
{
    std::vector<std::string> command;

    switch (destination.microcontroller()) {
        case Destination_Microcontroller_PICO_1: {
            command = { "./test.sh" };
            // command = { "openocd", "-f", "/etc/remoteprog/raspberrypi-swd.cfg", "-f", "/etc/remoteprog/rp2040.cfg", "-c", "program " + payload_filename + "; " + (verify ? "verify; " : "") + "reset; exit" };
            break;
        }
        case Destination_Microcontroller_PICO_2:
            command = { "openocd", "-f", "/etc/remoteprog/raspberrypi-swd.cfg", "-f", "/etc/remoteprog/rp2350.cfg", "-c", "adapter speed 5000", "-c", "rp2350.dap.core1 cortex_m reset_config sysresetreq", "-c", "program " + payload_filename + "; " + (verify ? "verify; " : "") + "reset; exit" };
            break;
        case Destination_Microcontroller_AVR:
            throw std::runtime_error("Not implemented"); // TODO
        default:
            throw std::runtime_error("Unreachable code");
    }

    runner::execute(fd, command, debug_mode);
}

void upload(int fd, Request_FirmwareUpload const& req, bool debug_mode)
{
    std::string filename;
    if (req.payload_compressed())
        filename = uncompress_file(req.payload());
    else
        filename = save_file(req.payload());

    // find destination
    Destination destination;
    destination.set_microcontroller(Destination_Microcontroller_AUTO);
    if (req.has_destination())
        destination = req.destination();
    destination = find_destination(destination);

    // upload payload to microcontroller
    upload_payload(fd, destination, filename, req.verify(), debug_mode);

    // delete file
    unlink(filename.c_str());
}

}
