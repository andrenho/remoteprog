#include "llcomm.hh"

#include <cstdlib>
#include <pigpio.h>
#include <unistd.h>

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
    uint32_t time = reset.time_ms() ? reset.time_ms() * 1000 : 100000;
    usleep(time);
    gpioSetMode(RESET, PI_INPUT);
    gpioSetPullUpDown(RESET, PI_PUD_UP);
#endif
}

void i2c_configure(Request_I2CConfig const& cfg)
{
#ifndef SIMULATE_LL
    if (i2c_handle != -1)
        i2cClose(i2c_handle);
    i2c_handle = i2cOpen(1, cfg.device(), 0);
    if (i2c_handle < 0)
        throw std::runtime_error("Could not open i2c handle (error: " + std::to_string(i2c_handle));
#endif
}

void i2c_send(uint8_t const* msg, size_t msg_sz, uint8_t* response, size_t response_sz)
{
#ifndef SIMULATE_LL
    i2cWriteDevice(i2c_handle, (char* const) msg, msg_sz);
    i2cReadDevice(i2c_handle, (char *) response, response_sz);
#else
    for (uint8_t i = 0; i < response_sz; ++i)
        response[i] = rand();
#endif
}

void spi_configure(Request_SPIConfig const& spi)
{
#ifndef SIMULATE_LL
    if (spi_handle != -1)
        spiClose(spi_handle);
    unsigned baud = spi.baud() == 0 ? 1000000 : spi.baud();
    uint32_t flags = 0;
    if (spi.pha())
        flags |= 0b1;
    if (spi.pol())
        flags |= 0b10;
    spi_handle = spiOpen(0, baud, flags);
    if (spi_handle < 0)
        throw std::runtime_error("Could not open spi handle (error: " + std::to_string(spi_handle));
#endif
}

void spi_send(uint8_t const* msg, size_t msg_sz, uint8_t* response)
{
#ifndef SIMULATE_LL
    spiXfer(spi_handle, (char *) msg, (char *) response, msg_sz);
#else
    for (uint8_t i = 0; i < msg_sz; ++i)
        response[i] = rand();
#endif
}

void finalize()
{
#ifndef SIMULATE_LL
    if (i2c_handle != -1)
        i2cClose(i2c_handle);
    if (spi_handle != -1)
        spiClose(spi_handle);
#endif
}

}
