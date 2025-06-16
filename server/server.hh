#ifndef SERVER_HH
#define SERVER_HH

#include <optional>
#include <string>

namespace server {

constexpr const char* PORT = "9182";

std::optional<std::string> local_ip();
void listen(bool debug_mode);

};

#endif //SERVER_HH
