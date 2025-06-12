#include "client.hh"

#include <getopt.h>

#include <optional>
#include <string>

#include "lastcall.hh"
#include "options.hh"
#include "request.hh"

static void print_help(const char* program_name)
{
    printf(R"(Usage: %s COMMAND [OPTIONS]
Commands:
  upload FILE [-c|--core=CORE] [-p|--part=PART] [--baud=BAUD] [-V|--no-verify] [-v|--verbose]
  test [-c|--core=CORE] [-p|--part=PART]
  fuse LOW HIGH EXTENDED [-p|--part=PART]
  reset [-t|--time MS]
  spi [FILE] [--baud=BAUD] [--pol=POL] [--pha=PHA] [-I|--interactive]
  i2c [FILE] [--baud=BAUD] [-z|--response-sz=COUNT] [-I|--interactive]
General options:
  -s | --server         server address (will use latest if not present)
  -d | --debug          debug mode
Cores:
  auto, pico1, pico2, avr
)", program_name);
    exit(EXIT_SUCCESS);
}

static Options parse_options(int argc, char* argv[])
{
    Options opt {};

    static option options[] = {
        { "core",        optional_argument, nullptr, 'c' },
        { "part",        optional_argument, nullptr, 'p' },
        { "no-verify",   no_argument,       nullptr, 'V' },
        { "verbose",     no_argument,       nullptr, 'v' },
        { "time",        optional_argument, nullptr, 't' },
        { "baud",        optional_argument, nullptr, 'b' },
        { "pol",         optional_argument, nullptr, 'O' },
        { "pha",         optional_argument, nullptr, 'A' },
        { "interactive", no_argument,       nullptr, 'I' },
        { "response-sz", optional_argument, nullptr, 'z' },
        { "server",      optional_argument, nullptr, 's' },
        { "help",        no_argument,       nullptr, 'h' },
        { "debug",       no_argument,       nullptr, 'd' },
        { nullptr, 0, nullptr, 0 },
    };

    int idx = 0;

    while (true) {
        int c = getopt_long(argc, argv, "c:p:Vvt:b:O:A:Iz:s:d", options, &idx);
        if (c == -1)
            break;

        switch (c) {
            case 0: case 'h': print_help(argv[0]); break;
            case 'c': opt.core = optarg; break;
            case 'p': opt.part = optarg; break;
            case 'V': opt.verify = false; break;
            case 'v': opt.verbose = true; break;
            case 't': opt.wtime = std::stoi(optarg); break;
            case 'b': opt.baud = std::stoi(optarg); break;
            case 'O': opt.pol = std::stoi(optarg); break;
            case 'A': opt.pha = std::stoi(optarg); break;
            case 'I': opt.interactive = true; break;
            case 'z': opt.response_count = std::stoi(optarg); break;
            case 's': opt.server = optarg; break;
            case 'd': opt.debug_mode = true; break;
            case '?': break;
            default: throw std::runtime_error("Something went wrong.");
        }
    }

    while (optind < argc) {
        if (opt.command.empty()) {
            opt.command = argv[optind];
        } else if (opt.command == "upload" || opt.command == "i2c" || opt.command == "spi") {
            opt.file = argv[optind];
        } else if (opt.command == "fuse") {
            if (!opt.fuse_low)
                opt.fuse_low = std::stoi(argv[optind], nullptr, 16);
            else if (!opt.fuse_high)
                opt.fuse_high = std::stoi(argv[optind], nullptr, 16);
            else if (!opt.fuse_extended)
                opt.fuse_extended = std::stoi(argv[optind], nullptr, 16);
        }
        ++optind;
    }

    if (opt.command.empty())
        print_help(argv[0]);

    return opt;
}

static void update_last_data(Options& opt, lastcall::Data data)
{
    if (opt.server.empty())
        try { opt.server = data.at("server"); } catch (std::out_of_range&) {}
    else
        data["server"] = opt.server;
    if (opt.core.empty())
        try { opt.core = data.at("core"); } catch (std::out_of_range&) {}
    else
        data["core"] = opt.core;
    if (opt.part.empty())
        try { opt.part = data.at("part"); } catch (std::out_of_range&) {}
    else
        data["part"] = opt.part;
    lastcall::save(data);
}

int main(int argc, char* argv[])
{
    try {
        Options opt = parse_options(argc, argv);
        lastcall::Data last_data = lastcall::load();
        update_last_data(opt, last_data);

        if (opt.server.empty())
            throw std::runtime_error("A server configuration was not found. Please determine the server.");

        Request request = build_request(opt);

        client::connect(opt.server);
        Response response = client::send_request(request, opt.debug_mode);

        if (response.response_case() == Response::kResult) {
            again:
                    if (!response.result().messages().empty())
                        printf("%s\n", response.result().messages().c_str());
            if (!response.result().errors().empty())
                fprintf(stderr, "\e[0;31m%s\e[0m\n", response.result().errors().c_str());
            switch (response.result().result_code()) {
                case Response_ResultCode_SUCCESS:
                    return EXIT_SUCCESS;
                case Response_ResultCode_FAILURE:
                    return EXIT_FAILURE;
                case Response_ResultCode_ONGOING:
                    response = client::wait_for_next_message(opt.debug_mode);
                    goto again;
                default: break;
            }
        }

        // TODO - other responses

        return 0;
    } catch (std::exception& e) {
        fprintf(stderr, "client error: %s\n", e.what());
        exit(EXIT_FAILURE);
    }
}