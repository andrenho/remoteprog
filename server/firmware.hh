#ifndef FIRMWARE_HH
#define FIRMWARE_HH

#include "messages.pb.h"

namespace firmware {

void upload(int fd, Request_FirmwareUpload const& req, bool debug_mode);

}

#endif //FIRMWARE_HH
