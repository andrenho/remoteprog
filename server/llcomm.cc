#include "llcomm.hh"

#include <cstdlib>
#include <pigpio.h>
#include <unistd.h>

#define RESET 12

namespace llcomm {

void init()
{
#ifndef SIMULATE_LL
    if (gpioInitialise() < 0)
        throw std::runtime_error("Could not initialize pigpio.");

    gpioSetMode(RESET, PI_INPUT);
    gpioSetPullUpDown(RESET, PI_PUD_UP);
#endif
}

void close()
{
#ifndef SIMULATE_LL
    gpioTerminate();
#endif
}

void reset(Request_Reset const& reset)
{
#ifndef SIMULATE_LL
    gpioSetPullUpDown(RESET, PI_PUD_OFF);
    gpioSetMode(RESET, PI_OUTPUT);
    gpioWrite(RESET, 0);
    uint32_t time = reset.time_ms() ? reset.time_ms() * 1000 : 100000;
    usleep(time);
    gpioSetMode(RESET, PI_INPUT);
    gpioSetPullUpDown(RESET, PI_PUD_UP);
#endif
}

}
