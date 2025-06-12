#ifndef FIRMWARE_HH
#define FIRMWARE_HH

#include "messages.pb.h"

namespace firmware {

void upload(int fd, Request_FirmwareUpload const& req, bool debug_mode);
void test_connection(int fd, Destination const& dest, bool debug_mode);
void program_fuses(int fd, Request_AvrFuseProgramming const& fuses, bool debug_mode);

}

#endif //FIRMWARE_HH
