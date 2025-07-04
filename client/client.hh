#ifndef CLIENT_HH
#define CLIENT_HH

#include "messages.pb.h"

namespace client {

constexpr const char* PORT = "9182";

void connect(std::string const& server_addr);
Response send_request(Request const& request, bool debug_mode);
Response wait_for_next_message(bool debug_mode);

}

#endif //CLIENT_HH
