#ifndef FIRMWARE_HH
#define FIRMWARE_HH

#include "messages.pb.h"

namespace firmware {

bool upload(int fd, Request_FirmwareUpload const& req, bool debug_mode);
bool test_connection(int fd, Destination const& dest, bool debug_mode);
bool program_fuses(int fd, Request_AvrFuseProgramming const& fuses, bool debug_mode);

}

#endif //FIRMWARE_HH
