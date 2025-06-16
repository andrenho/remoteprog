#include "llcomm.hh"

#include <cstdlib>
#include <pigpio.h>

#define RESET 12

namespace llcomm {

static int i2c_handle = -1;
static int spi_handle = -1;

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
    usleep(100000);
    gpioSetMode(RESET, PI_INPUT);
    gpioSetPullUpDown(RESET, PI_PUD_UP);
#endif
}

void i2c_configure(Request_I2CConfig const& cfg)
{
#ifndef SIMULATE_LL
    if (i2c_handle == -1)
        i2cClose(i2c_handle);
    i2c_handle = i2cOpen(1, cfg.device(), 0);
    // TODO
#endif
}

void i2c_send(uint8_t const* msg, size_t msg_sz, uint8_t* response, size_t response_sz)
{
#ifndef SIMULATE_LL
    // TODO
#else
    for (uint8_t i = 0; i < response_sz; ++i)
        response[i] = rand();
#endif
}

void spi_configure(Request_SPIConfig const& spi)
{
#ifndef SIMULATE_LL
    // TODO
#endif
}

void spi_send(uint8_t const* msg, size_t msg_sz, uint8_t* response)
{
#ifndef SIMULATE_LL
    // TODO
#else
    for (uint8_t i = 0; i < msg_sz; ++i)
        response[i] = rand();
#endif
}

void finalize()
{
#ifndef SIMULATE_LL
    // TODO
#endif
}

}
