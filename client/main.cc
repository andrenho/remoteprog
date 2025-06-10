#include "client.hh"

#include <getopt.h>
#include <string>

static std::string command, core = "auto", part, file, server;
static int         verify = 1, verbose, interactive;
static uint8_t     fuse_low, fuse_high, fuse_extended, pol, pha;
static uint32_t    baud, response_count, wtime;

static void print_help(const char* program_name)
{
    printf(R"(Usage: %s COMMAND [OPTIONS]
Commands:
  upload FILE [-c|--core=CORE] [-p|--part=PART] [-V|--no-verify] [-v|--verbose]
  test [-c|--core=CORE] [-p|--part=PART]
  fuse LOW HIGH EXTENDED [-p|--part=PART]
  reset [-t|--time MS]
  spi [FILE] [--baud=BAUD] [--pol=POL] [--pha=PHA] [-I|--interactive]
  i2c [FILE] [--baud=BAUD] [-z|--response-sz=COUNT] [-I|--interactive]
General options:
  -s | --server         server address (will use latest if not present)
)", program_name);
}

static void parse_options(int argc, char* argv[])
{
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
        { nullptr, 0, nullptr, 0 },
    };

    int idx = 0;

    while (true) {
        int c = getopt_long(argc, argv, "c:p:Vvt:b:O:A:Iz:s:", options, &idx);
        if (c == -1)
            return;

        switch (c) {
            case 0: case 'h': print_help(argv[0]); break;
            case 'c': core = optarg; break;
            case 'p': part = optarg; break;
            case 'V': verify = false; break;
            case 'v': verbose = true; break;
            case 't': wtime = std::stoi(optarg); break;
            case 'b': baud = std::stoi(optarg); break;
            case 'O': pol = std::stoi(optarg); break;
            case 'A': pha = std::stoi(optarg); break;
            case 'I': interactive = true; break;
            case 'z': response_count = std::stoi(optarg); break;
            case 's': server = optarg; break;
            case '?': break;
            default: throw std::runtime_error("Something went wrong.");
        }
    }
}

int main(int argc, char* argv[])
{
    parse_options(argc, argv);

    client::connect("localhost");

    Request request;
    request.set_ack(true);

    Response response = client::send_request(request);

    if (response.response_case() == Response::kResult && response.result().success())
        printf("Success\n");
    else
        printf("Failure\n");
}