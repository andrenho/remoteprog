#include "server.hh"

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

#include <stdexcept>
#include <string>

#include "firmware.hh"
#include "llcomm.hh"
using namespace std::string_literals;

#include "messages.pb.h"
#include "../protobuf/comm.hh"
#include "ui.hh"

static int socket_fd = -1;

namespace server {

static bool handle(int fd, bool debug_mode);
static void send_error(int fd, std::string const& error_msg, bool debug_mode);
static void send_success(int fd, bool debug);

std::optional<std::string> local_ip()
{
    int sock = socket(PF_INET, SOCK_DGRAM, 0);
    sockaddr_in loopback {};

    if (sock == -1)
        return {};

    std::memset(&loopback, 0, sizeof(loopback));
    loopback.sin_family = AF_INET;
    loopback.sin_addr.s_addr = 1337;   // can be any IP address
    loopback.sin_port = htons(9);      // using debug port

    if (connect(sock, reinterpret_cast<sockaddr*>(&loopback), sizeof(loopback)) == -1)
        return {};

    socklen_t addrlen = sizeof(loopback);
    if (getsockname(sock, reinterpret_cast<sockaddr*>(&loopback), &addrlen) == -1)
        return {};

    close(sock);

    char buf[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &loopback.sin_addr, buf, INET_ADDRSTRLEN) == nullptr)
        return {};

    return buf;
}

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
    addrinfo* p;
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
        bool running = true;
        while (running) {
            try {
                running = handle(conn_fd, debug_mode);
            } catch (std::exception& e) {
                fprintf(stderr, "error: %s\n", e.what());
                send_error(conn_fd, e.what(), debug_mode);
            }
        }
        printf("Connection closed.\n");
    }
}

static bool handle(int fd, bool debug_mode)
{
    // receive request
    Request request;
    try {
        auto r = wait_for_message<Request>(fd, debug_mode);
        if (!r) {   // client closed connection
            close(fd);
            return false;
        }
        request = *r;
    } catch (std::exception&) {
        send_error(fd, "Invalid protobuf message", debug_mode);
        close(fd);
        return true;
    }

    ui::set_position(0, 1);
    ui::print("                ");
    ui::set_position(0, 1);

    // act on message
    try {
        switch (request.request_case()) {
            case Request::kFirmwareUpload:
                ui::print("Uploading...");
                ui::set_position(0, 1);
                if (firmware::upload(fd, request.firmware_upload(), debug_mode))
                    ui::print("Done.       ");
                else
                    ui::print("ERROR       ");
                break;
            case Request::kTestConnection:
                ui::print("Testing...");
                ui::set_position(0, 1);
                if (firmware::test_connection(fd, request.test_connection(), debug_mode))
                    ui::print("Done.     ");
                else
                    ui::print("ERROR     ");
                break;
            case Request::kFuseProgramming:
                ui::print("Programming...");
                ui::set_position(0, 1);
                if (firmware::program_fuses(fd, request.fuse_programming(), debug_mode))
                    ui::print("Done.         ");
                else
                    ui::print("ERROR         ");
                break;
            case Request::kReset:
                llcomm::reset(request.reset());
                send_success(fd, debug_mode);
                break;
            case Request::kSpiConfig:
                llcomm::spi_configure(request.spi_config());
                send_success(fd, debug_mode);
                break;
            case Request::kSpiMessage: {
                ui::print("SPI");
                uint8_t response[request.spi_message().size()];
                llcomm::spi_send((uint8_t const*) request.spi_message().data(), request.spi_message().size(), response);
                Response r;
                r.set_message(std::string((const char *) response, request.spi_message().size()));
                send_message(fd, r, debug_mode);
                break;
            }
            case Request::kI2CConfig:
                llcomm::i2c_configure(request.i2c_config());
                send_success(fd, debug_mode);
                break;
            case Request::kI2CMessage:{
                ui::print("I2C");
                uint8_t response[request.i2c_message().expect_response_sz()];
                llcomm::i2c_send((uint8_t const*) request.i2c_message().data().data(), request.i2c_message().data().size(), response, request.i2c_message().expect_response_sz());
                Response r;
                r.set_message(std::string((const char *) response, request.i2c_message().expect_response_sz()));
                send_message(fd, r, debug_mode);
                break;
            }
            case Request::kFinalize:
                llcomm::finalize();
                send_success(fd, debug_mode);
                break;
            case Request::REQUEST_NOT_SET:
                send_error(fd, "Protobuf message without a request", debug_mode);
            close(fd);
        }
    } catch (std::exception& e) {
        send_error(fd, e.what(), debug_mode);
        ui::set_position(0, 1);
        ui::print("ERROR           ");
    }

    return true;
}

static void send_success(int fd, bool debug)
{
    auto result = new Response_Result();
    result->set_result_code(Response_ResultCode_SUCCESS);

    Response response;
    response.set_allocated_result(result);

    send_message(fd, response, debug);
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
