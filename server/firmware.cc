#include "firmware.hh"

#include <format>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>

#include "runner.hh"
#include "zlib.h"
#include "messages.pb.h"

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

static Destination_Microcontroller auto_detect_microcontroller(std::string const& filename, bool debug_mode)
{
    int rp2350 = system(("strings " + filename + " | grep gpio_function_rp2350").c_str());
    int rp2040 = system(("strings " + filename + " | grep gpio_function_rp2040").c_str());
    int hex = system(("grep '^:[0-9A-F]\\+' " + filename).c_str());

    if (rp2350 == 0) {
        if (debug_mode) printf("Payload auto-detected as pico2.\n");
        return Destination_Microcontroller_PICO_2;
    }
    if (rp2040 == 0) {
        if (debug_mode) printf("Payload auto-detected as pico1.\n");
        return Destination_Microcontroller_PICO_1;
    }
    if (hex == 0) {
        if (debug_mode) printf("Payload auto-detected as AVR (hex).\n");
        return Destination_Microcontroller_AVR;
    }

    throw std::runtime_error("Could not auto-detect the file type.");
}

static Destination find_destination(Destination destination, std::string const& filename, bool debug_mode)
{
    if (destination.microcontroller() == Destination_Microcontroller_AUTO)
        destination.set_microcontroller(auto_detect_microcontroller(filename, debug_mode));

    if (destination.microcontroller() == Destination_Microcontroller_AVR && destination.part().empty())
        throw std::runtime_error("AVR require a part (ex. atmega328).");

    return destination;
}

static void upload_payload(int fd, Destination const& destination, std::string const& payload_filename, bool verify, bool debug_mode)
{
    std::vector<std::string> command;

    switch (destination.microcontroller()) {
        case Destination_Microcontroller_PICO_1: {
            command = { "openocd", "-f", "/etc/remoteprog/raspberrypi-swd.cfg", "-f", "/etc/remoteprog/rp2040.cfg", "-c", "program " + payload_filename + "; " + (verify ? "verify; " : "") + "reset; exit" };
            break;
        }
        case Destination_Microcontroller_PICO_2:
            command = { "openocd", "-f", "/etc/remoteprog/raspberrypi-swd.cfg", "-f", "/etc/remoteprog/rp2350.cfg", "-c", "adapter speed 5000", "-c", "rp2350.dap.core1 cortex_m reset_config sysresetreq", "-c", "program " + payload_filename + "; " + (verify ? "verify; " : "") + "reset; exit" };
            break;
        case Destination_Microcontroller_AVR:
            command = { "avrdude", "-p", destination.part(), "-C", "/etc/remoteprog/avrdude.conf", "-c", "remoteprog", "-U", "flash:w:" + payload_filename + ":i" };
            if (!verify)
                command.emplace_back("-V");
            break;
        default:
            throw std::runtime_error("Unreachable code");
    }

    runner::execute(fd, command, debug_mode);
}

void test_connection(int fd, Destination const& dest, bool debug_mode)
{
    std::vector<std::string> command;

    switch (dest.microcontroller()) {
        case Destination_Microcontroller_PICO_1:
        case Destination_Microcontroller_PICO_2:
            command = { "openocd", "-f", "/etc/remoteprog/raspberrypi-swd.cfg", "-f", "/etc/remoteprog/rp2350.cfg", "-c", "adapter speed 5000", "reset; exit" };
            break;
        case Destination_Microcontroller_AVR:
            command = { "avrdude", "-p", dest.part(), "-C", "/etc/remoteprog/avrdude.conf", "-c", "remoteprog" };
            break;
        default:
            throw std::runtime_error("Unreachable code");
    }

    runner::execute(fd, command, debug_mode);
}

void program_fuses(int fd, Request_AvrFuseProgramming const& fuses, bool debug_mode)
{
    std::vector<std::string> command;
    if (fuses.destination().microcontroller() == Destination_Microcontroller_AUTO || fuses.destination().microcontroller() == Destination_Microcontroller_AVR) {
        command = {
            "avrdude", "-p", fuses.destination().part(), "-C", "/etc/remoteprog/avrdude.conf", "-c", "remoteprog",
            "-U", std::format("lfuse:w:0x{:x}:m", fuses.low()),
            "-U", std::format("hfuse:w:0x{:x}:m", fuses.high())
        };
        if (fuses.has_extended()) {
            command.emplace_back("-U");
            command.emplace_back(std::format("efuse:w:0x{:x}:m", fuses.extended()));
        }
    } else {
        throw std::runtime_error("Fuse programming is only supported for AVR.");
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
    destination = find_destination(destination, filename, debug_mode);

    // upload payload to microcontroller
    upload_payload(fd, destination, filename, req.verify(), debug_mode);

    // delete file
    unlink(filename.c_str());
}

}
