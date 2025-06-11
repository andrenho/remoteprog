#ifndef COMM_HH
#define COMM_HH

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <optional>
#include <stdexcept>
#include <string>
using namespace std::string_literals;

#include "../client/messages.pb.h"

template <typename T>
void send_message(int fd, T const& message)
{
    std::string data = message.SerializeAsString();
    uint32_t sz = data.size();

    // send response header
    uint8_t hbuf[6] = {
        0xf1, 0xf0,
        (uint8_t) (sz & 0xff), (uint8_t) ((sz >> 8) & 0xff), (uint8_t) ((sz >> 16) & 0xff), (uint8_t) ((sz >> 24) & 0xff)
    };
#ifdef DEBUG_MESSAGES
    printf("Sending message:\n");
    for (size_t i = 0; i < 6; ++i) printf("[\e[0;32m%02X\e[0m]", hbuf[i]);
#endif
    ssize_t n = send(fd, hbuf, 6, 0);
    if (n <= 0)
        throw std::runtime_error("Error sending header: "s + strerror(errno));
    if (n != 6)
        throw std::runtime_error("Client did not accept whole header.");

    // send response
    size_t i = 0;
    do {
        n = send(fd, &data[i], data.size() - i, 0);
#ifdef DEBUG_MESSAGES
        for (size_t j = i; j < data.size() - i; ++j) printf("[\e[0;32m%02X\e[0m]", (uint8_t) data[j]);
        printf("\n");
#endif
        if (n < 0)
            throw std::runtime_error("send(): "s + strerror(errno));
        i += n;
    } while (i < data.size());
}

template <typename T>
std::optional<T> wait_for_message(int fd)
{
    // receive response
    uint8_t hbuf[6];
    int n = recv(fd, hbuf, 6, MSG_WAITALL);
    if (n != 6)
        throw std::runtime_error("Error receiving header from client.");
#ifdef DEBUG_MESSAGES
    printf("Receiving message:\n");
    for (size_t i = 0; i < 6; ++i) printf("[\e[0;34m%02X\e[0m]", hbuf[i]);
#endif
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
    if (n == 0)
        return {};
    else if (n < 0)
        throw std::runtime_error("recv(): "s + strerror(errno));
#ifdef DEBUG_MESSAGES
        for (size_t j = 0; j < msg_sz; ++j) printf("[\e[0;34m%02X\e[0m]", (uint8_t) buf[j]);
        printf("\n");
#endif

    // parse message
    T message;
    if (!message.ParseFromArray(buf, msg_sz))
        throw std::runtime_error("Invalid protobuf message");
#ifdef DEBUG_MESSAGES
    printf("\e[0;34m%s\e[0m", message.DebugString().c_str());
#endif

    return message;
}

#endif //COMM_HH
