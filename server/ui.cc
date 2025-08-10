#include <cstdlib>
#include <pigpio.h>
#include <unistd.h>

#include <stdexcept>

extern "C" {
#include "ssd1306-driver/ssd1306.h"
#include "ssd1306-driver/toshiba_font.h"
}

#define BEEP 16

static int H;  // handle
static SSD_Buffer* bf;
               
static void i2c_init(void* data)
{
    int bus = (int) data;
    H = i2cOpen(bus, 0x3c, 0);
}

static void i2c_finalize(void* data)
{
    (void) data;
    i2cClose(H);
}

static void i2c_send_bytes(void* data, uint8_t const* bytes, size_t sz)
{
    (void) data;
    i2cWriteDevice(H, (char *) bytes, sz);

    /*
    for (size_t i = 0; i < sz; ++i)
        printf("%02X ", bytes[i]);
    printf("\n");
    */
}

namespace ui {

static void init_display()
{
    ssd1306_init((I2CFunctions) {
        .data = (void *) 1,
        .init = i2c_init,
        .send_bytes = i2c_send_bytes,
        .finalize = i2c_finalize,
    }, 32);

    bf = ssd1306_create_buffer();
}

void init()
{
    gpioSetMode(BEEP, PI_OUTPUT);
    init_display();
}

void beep_success()
{
    gpioSetPWMfrequency(BEEP, 1000);
    gpioPWM(BEEP, 128);
    usleep(100000);
    gpioPWM(BEEP, 0);
}

void beep_error()
{
    gpioSetPWMfrequency(BEEP, 100);
    gpioPWM(BEEP, 128);
    usleep(500000);
    gpioPWM(BEEP, 0);
    usleep(200000);
    gpioPWM(BEEP, 128);
    usleep(500000);
    gpioPWM(BEEP, 0);
}


void print(const char* str)
{
    // TODO
}

void clear()
{
    // TODO
}

void set_position(uint8_t x, uint8_t y)
{
    /*
    if (y == 0)
        SSD1306_SetPosition(7, 1);
    */
    // TODO
}

}
