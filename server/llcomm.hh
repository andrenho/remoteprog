#ifndef LLCOMM_HH
#define LLCOMM_HH

#include "messages.pb.h"

namespace llcomm {

void reset(Request_Reset const& reset);

void i2c_configure(Request_I2CConfig const& cfg);
void i2c_send(uint8_t const* msg, size_t msg_sz, uint8_t* response, size_t response_sz);

void spi_configure(Request_SPIConfig const& spi);
void spi_send(uint8_t const* msg, size_t msg_sz, uint8_t* response);

void finalize();

}

#endif //LLCOMM_HH
