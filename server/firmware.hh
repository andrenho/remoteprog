#ifndef FIRMWARE_HH
#define FIRMWARE_HH

#include "messages.pb.h"

namespace firmware {

void upload(int fd, Request_FirmwareUpload const& req);

}

#endif //FIRMWARE_HH
