#include "server.hh"

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <poll.h>

#include <stdexcept>
#include <string>
using namespace std::string_literals;

#include "messages.pb.h"

static int socket_fd = -1;

namespace server {

static void handle(int fd);

void listen()
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

    // loop through all the results and bind to the first we can
    addrinfo* p = nullptr;
    for (p = server_info; p != nullptr; p = p->ai_next) {

        // create a socket, which apparently is no good by itself because it's not
        // bound to an address and port number
        socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (socket_fd == -1) {
            perror("socket()");
            continue;
        }

        // lose the "Address already in use" error message. why this happens
        // in the first place? well even after the server is closed, the port
        // will still be hanging around for a while, and if you try to restart
        // the server, you'll get an "Address already in use" error message
        int yes = 1;
        rv = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
        if (rv == -1)
            throw std::runtime_error("setsockopt(): "s + strerror(errno));

        // bind the socket to the address and port number
        rv = bind(socket_fd, p->ai_addr, p->ai_addrlen);
        if (rv == -1) {
            close(socket_fd);
            perror("bind()");
            continue;
        }

        // binding was successful, so break out of the loop
        break;
    }

    freeaddrinfo(server_info);

    // check if we were able to bind to an address and port number
    if (!p)
        throw std::runtime_error("failed to bind");

    // time to listen for incoming connections
    // can only accept one incoming connection
    rv = ::listen(socket_fd, 1);
    if (rv == -1)
        throw std::runtime_error("listen(): "s + strerror(errno));

    printf("Server listening in port %s...\n", PORT);

    // wait for new connections
    sockaddr_storage client_addr {};
    socklen_t addr_size = sizeof client_addr;

    int conn_fd = accept(socket_fd, (sockaddr *) &client_addr, &addr_size);
    if (conn_fd == -1)
        throw std::runtime_error("accept(): "s + strerror(errno));

    handle(conn_fd);
}

static void handle(int fd)
{
    printf("Connection received.\n");

    // receive header
    uint8_t hbuf[6];
    ssize_t n = recv(fd, hbuf, 6, MSG_WAITALL);
    if (n != 6)
        throw std::runtime_error("Error receiving header from client.");
    if (hbuf[0] != 'a' || hbuf[1] != 'b')
        throw std::runtime_error("Invalid header");
    uint32_t msg_sz = hbuf[2]
                    | ((uint32_t) hbuf[3]) << 8
                    | ((uint32_t) hbuf[4]) << 16
                    | ((uint32_t) hbuf[5]) << 24;
    printf("Receiving message with %zu bytes.\n", msg_sz);

    // receive message
    uint8_t buf[msg_sz];
    n = recv(fd, buf, msg_sz, MSG_WAITALL);
}

}

// https://github.com/biraj21/tcp-server/blob/main/src/server.c