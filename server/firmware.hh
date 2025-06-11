#ifndef FIRMWARE_HH
#define FIRMWARE_HH

#include "messages.pb.h"

namespace firmware {

Response_Result* upload(Request_FirmwareUpload const& req);

}

#endif //FIRMWARE_HH
