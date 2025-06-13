#include "interactive.hh"

#include <fstream>

#include "client.hh"
#include "messages.pb.h"

namespace interactive {

void process_interactive(Options const& opt, bool debug_mode)
{
}

void process_noninteractive(Options const& opt, bool debug_mode)
{
    if (opt.file.empty())
        throw std::runtime_error("File was not provided");

    if (opt.command == "i2c" && !opt.response_count)
        throw std::runtime_error("`--response-sz` or `-z` parameter is required for i2c message.");

    std::ifstream f(opt.file);
    if (!f.is_open())
        throw std::runtime_error("Could not open file " + opt.file);
    std::string content((std::istreambuf_iterator(f)), std::istreambuf_iterator<char>());
    f.close();

    Request request;
    if (opt.command == "spi") {
        request.set_spi_message(content);
    } else if (opt.command == "i2c") {
        auto msg = new Request_I2CMessage;
        msg->set_expect_response_sz(*opt.response_count);
        msg->set_data(content);
        request.set_allocated_i2c_message(msg);
    }

    Response response = client::send_request(request, debug_mode);
    if (!response.has_message())
        throw std::runtime_error("Unexpected response from server");
    for (uint8_t c : response.message())
        printf("%02X ", c);
    printf("\n");
}

void process(Options const& opt, bool debug_mode)
{
    if (opt.interactive)
        process_interactive(opt, debug_mode);
    else
        process_noninteractive(opt, debug_mode);
}

}
