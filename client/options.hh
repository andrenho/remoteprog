#ifndef OPTIONS_HH
#define OPTIONS_HH

#include <cstdint>
#include <string>
#include <optional>

struct Options
{
    std::string             command, core = "auto", part, file, server;
    bool                    verify = true, verbose, debug_mode;
    std::optional<uint8_t>  fuse_low, fuse_high, fuse_extended;
    std::optional<uint32_t> baud, wtime;
};

#endif //OPTIONS_HH
