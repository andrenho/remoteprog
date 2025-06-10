#ifndef CLIENT_HH
#define CLIENT_HH

#include "messages.pb.h"

namespace client {

constexpr const char* PORT = "9182";

void connect(std::string const& server_addr);
Response send_request(Request const& request);
Response wait_for_next_response();

}

#endif //CLIENT_HH
