#include "server.hh"

#include <cstring>
#include <cstdlib>
#include <ctime>

#include "llcomm.hh"
#include "ui.hh"

int main(int argc, char* argv[])
{
    srand(time(nullptr));

    bool debug_mode = false;
    if (argc == 2 && strcmp(argv[1], "-d") == 0)
        debug_mode = true;

    llcomm::init();
    auto ip = server::local_ip();

    ui::init();
    ui::set_position(0, 0);
    if (ip)
        ui::print(ip->c_str());
    else
        ui::print("No network");
    ui::beep_success();
    server::listen(debug_mode);

    llcomm::close();  // TODO - catch SIGINT
}
