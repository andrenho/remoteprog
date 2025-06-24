#include "interactive.hh"

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "client.hh"
#include "messages.pb.h"

namespace interactive {

static void send_message_and_print(Options const& opt, std::string bytes, int response_count, bool debug_mode)
{
    Request request;
    if (opt.command == "spi") {
        request.set_spi_message(bytes);
    } else if (opt.command == "i2c") {
        auto msg = new Request_I2CMessage;
        msg->set_expect_response_sz(response_count);
        msg->set_data(bytes);
        request.set_allocated_i2c_message(msg);
    }

    Response response = client::send_request(request, debug_mode);
    if (!response.has_message())
        throw std::runtime_error("Unexpected response from server");
    for (uint8_t c : response.message())
        printf("%02X ", c);
    printf("\n");
}

void process_interactive(Options const& opt, bool debug_mode)
{
    printf("(CTRL+D to stop)\n");

    for (;;) {
        printf("Bytes? ");

        std::string line;
        std::vector<uint8_t> bytes;
        std::string hex;

        //std::cin.ignore();
        if (!std::getline(std::cin, line))
            exit(EXIT_SUCCESS);

        std::istringstream iss(line);
        while (iss >> hex)
            bytes.push_back((uint8_t) std::stoul(hex, nullptr, 16));

        int response_count = 0;
        if (opt.command == "i2c") {
            printf("Response size? ");
            scanf("%d", &response_count);
        }

        send_message_and_print(opt, std::string(bytes.begin(), bytes.end()), response_count, debug_mode);
    }
}

void process_noninteractive(Options const& opt, bool debug_mode)
{
    if (opt.file.empty())
        throw std::runtime_error("File was not provided");

    std::ifstream f(opt.file);
    if (!f.is_open())
        throw std::runtime_error("Could not open file " + opt.file);
    std::string content((std::istreambuf_iterator(f)), std::istreambuf_iterator<char>());
    f.close();

    send_message_and_print(opt, content, opt.response_count.value_or(0), debug_mode);
}

void send_config(Options const& opt, bool debug_mode)
{
    Request req;
    if (opt.command == "spi") {
        auto spi_config = new Request_SPIConfig;
        if (opt.baud) spi_config->set_baud(*opt.baud);
        if (opt.pha) spi_config->set_pha(*opt.pha);
        if (opt.pol) spi_config->set_pha(*opt.pol);
        req.set_allocated_spi_config(spi_config);
    } else if (opt.command == "i2c") {
        auto i2c_config = new Request_I2CConfig;
        if (!opt.device_id)
            throw std::runtime_error("A device id is required.");
        i2c_config->set_device(*opt.device_id);
        req.set_allocated_i2c_config(i2c_config);
    }

    Response response = client::send_request(req, debug_mode);
    if (response.response_case() == Response::kResult) {
        if (response.result().result_code() == Response_ResultCode_FAILURE)
            throw std::runtime_error("There was an error configuring the protocol.");
    } else {
        throw std::runtime_error("Unexpected response type");
    }
}

void process(Options const& opt, bool debug_mode)
{
    send_config(opt, debug_mode);

    if (opt.interactive)
        process_interactive(opt, debug_mode);
    else
        process_noninteractive(opt, debug_mode);
}

}
