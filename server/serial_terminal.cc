#include "serial_terminal.hh"

#include <fstream>
#include <sstream>
#include <string>

#include "pigpio.h"

namespace serial_terminal {

void init()
{
    gpioSetPullUpDown(23, PI_PUD_UP);
    gpioSetAlertFunc(23, [](int gpio, int level, uint32_t tick) {
        printf("Change detected level %d\n", level);
    });
}

bool get()
{
    std::stringstream buffer;
    std::ifstream f("/boot/firmware/cmdline.txt");
    buffer << f.rdbuf();
    std::string text = buffer.str();

    return text.find("serial0") != std::string::npos;
}

void set(bool value)
{
}

}
