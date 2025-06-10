#include "client.hh"

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <stdexcept>
#include <string>
using namespace std::string_literals;

#include "messages.pb.h"

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
    int rv = getaddrinfo(nullptr, PORT, &hints, &server_info);
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

Response send_request(Request const& request)
{
    std::string data = request.SerializeAsString();
    uint32_t sz = data.size();

    // send request header
    uint8_t hbuf[6] = {
        0xf1, 0xf0,
        (uint8_t) (sz & 0xff), (uint8_t) ((sz >> 8) & 0xff), (uint8_t) ((sz >> 16) & 0xff), (uint8_t) ((sz >> 24) & 0xff)
    };
    ssize_t n = send(fd, hbuf, 6, 0);
    if (n <= 0)
        throw std::runtime_error("Error sending header: "s + strerror(errno));
    if (n != 6)
        throw std::runtime_error("Server did not accept whole header.");

    // send message
    size_t i = 0;
    do {
        n = send(fd, &data[i], data.size() - i, 0);
        if (n < 0)
            throw std::runtime_error("send(): "s + strerror(errno));
        i += n;
    } while (i < data.size());

    wait_for_next_response();
}

Response wait_for_next_response()
{
    // receive response
    uint8_t hbuf[6];
    int n = recv(fd, hbuf, 6, MSG_WAITALL);
    if (n != 6)
        throw std::runtime_error("Error receiving header from client.");
    if (hbuf[0] != 0xf1 || hbuf[1] != 0xf0)
        throw std::runtime_error("Invalid header");
    uint32_t msg_sz = hbuf[2]
                    | ((uint32_t) hbuf[3]) << 8
                    | ((uint32_t) hbuf[4]) << 16
                    | ((uint32_t) hbuf[5]) << 24;

    usleep(0);  // not sure why we need this

    // receive message
    char buf[msg_sz];
    n = recv(fd, buf, msg_sz, MSG_WAITALL);
    if (n <= 0)
        throw std::runtime_error("recv(): "s + strerror(errno));

    // parse message
    Response response;
    if (!response.ParseFromString(buf))
        throw std::runtime_error("Invalid protobuf message");

    return response;
}

}
