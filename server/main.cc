#include "server.hh"

#include <cstring>

int main(int argc, char* argv[])
{
    bool debug_mode = false;
    if (argc == 2 && strcmp(argv[1], "-d") == 0)
        debug_mode = true;

    server::listen(debug_mode);
}