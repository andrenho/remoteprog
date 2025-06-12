#include "server.hh"

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <stdexcept>
#include <string>

#include "firmware.hh"
using namespace std::string_literals;

#include "messages.pb.h"
#include "../protobuf/comm.hh"

static int socket_fd = -1;

namespace server {

static void handle(int fd, bool debug_mode);
static void send_error(int fd, std::string const& error_msg, bool debug_mode);

void listen(bool debug_mode)
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

    while (true) {
        int conn_fd = accept(socket_fd, (sockaddr *) &client_addr, &addr_size);
        if (conn_fd == -1)
            throw std::runtime_error("accept(): "s + strerror(errno));
        printf("Connection received.\n");
        handle(conn_fd, debug_mode);
    }
}

static void handle(int fd, bool debug_mode)
{
    // receive request
    Request request;
    try {
        auto r = wait_for_message<Request>(fd, debug_mode);
        if (!r) {
            close(fd);
            return;  // client disconnected
        }
        request = *r;
    } catch (std::exception& e) {
        send_error(fd, "Invalid protobuf message", debug_mode);
        close(fd);
        return;
    }

    Response response;

    // act on message
    switch (request.request_case()) {
        case Request::kFirmwareUpload: {
            firmware::upload(fd, request.firmware_upload(), debug_mode);
            goto skip_message;
        }
        case Request::kTestConnection:
            break;
        case Request::kFuseProgramming:
            break;
        case Request::kReset: {
            auto result = new Response_Result();
            result->set_result_code(Response_ResultCode_SUCCESS);
            result->set_messages("Microcontroller reset");
            response.set_allocated_result(result);
            // TODO
            break;
        }
        case Request::kSpiConfig:
            break;
        case Request::kSpiMessage:
            break;
        case Request::kI2CConfig:
            break;
        case Request::kI2CMessage:
            break;
        case Request::kFinalize:
            break;
        case Request::REQUEST_NOT_SET:
            send_error(fd, "Protobuf message without a request", debug_mode);
            close(fd);
            return;
    }

    send_message(fd, response, debug_mode);

skip_message:
    handle(fd, debug_mode);  // next message
}

static void send_error(int fd, std::string const& error_msg, bool debug_mode)
{
    auto result = new Response_Result();
    result->set_result_code(Response_ResultCode_FAILURE);
    result->set_errors(error_msg);

    Response response;
    response.set_allocated_result(result);

    send_message(fd, response, debug_mode);
}

}

// https://github.com/biraj21/tcp-server/blob/main/src/server.c