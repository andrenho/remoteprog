#ifndef OPTIONS_HH
#define OPTIONS_HH

#include <cstdint>
#include <string>
#include <optional>

struct Options
{
    std::string             command, core = "auto", part, file, server;
    bool                    verify = true, verbose, interactive, debug_mode;
    std::optional<uint8_t>  fuse_low, fuse_high, fuse_extended, pol, pha, device_id;
    std::optional<uint32_t> baud, response_count, wtime;
};

#endif //OPTIONS_HH
