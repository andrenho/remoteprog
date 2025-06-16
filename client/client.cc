#include "client.hh"

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <stdexcept>
#include <string>
using namespace std::string_literals;

#include "messages.pb.h"
#include "comm.hh"

namespace client {

static int fd;

void connect(std::string const& server_addr)
{
    addrinfo hints {};
    hints.ai_family = AF_UNSPEC;     // don't care whether it's IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    // NULL to assign the address of my local host to socket structures
    addrinfo *server_info = nullptr;
    int rv = getaddrinfo(server_addr.c_str(), PORT, &hints, &server_info);
    if (rv != 0)
        throw std::runtime_error("getaddrinfo(): "s + gai_strerror(rv));

    fd = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    if (fd == -1)
        throw std::runtime_error("socket(): "s + strerror(errno));

    rv = connect(fd, server_info->ai_addr, server_info->ai_addrlen);
    if (rv != 0)
        throw std::runtime_error("getaddrinfo(): "s + strerror(errno));
    freeaddrinfo(server_info);
}

Response send_request(Request const& request, bool debug_mode)
{
    send_message(fd, request, debug_mode);
    auto r = wait_for_message<Response>(fd, debug_mode);
    if (!r)
        throw std::runtime_error("Connection closed.");
    return *r;
}

Response wait_for_next_message(bool debug_mode)
{
    auto r = wait_for_message<Response>(fd, debug_mode);
    if (!r)
        throw std::runtime_error("Connection closed.");
    return *r;
}

}
