#include "llcomm.hh"

#include <cstdlib>

namespace llcomm {

void reset(Request_Reset const& reset)
{
#ifndef SIMULATE_LL
    // TODO
#endif
}

void i2c_configure(Request_I2CConfig const& cfg)
{
#ifndef SIMULATE_LL
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
