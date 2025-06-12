#include "server.hh"

#include <cstring>
#include <cstdlib>
#include <ctime>

int main(int argc, char* argv[])
{
    srand(time(nullptr));

    bool debug_mode = false;
    if (argc == 2 && strcmp(argv[1], "-d") == 0)
        debug_mode = true;

    server::listen(debug_mode);
}