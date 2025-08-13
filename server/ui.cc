#include <cstdlib>
#include <cstring>
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
    (void) data;
    H = i2cOpen(1, 0x3c, 0);
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

static uint8_t pos_x = 0;
static uint8_t pos_y = 3;

static void init_display()
{
    ssd1306_init((I2CFunctions) {
        .data = nullptr,
        .init = i2c_init,
        .finalize = i2c_finalize,
        .send_bytes = i2c_send_bytes,
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
    buffer_fill_rect(bf, pos_x, pos_y, toshiba_font.char_w * strlen(str), toshiba_font.char_h, false);
    buffer_draw_string(bf, &toshiba_font, pos_x, pos_y, str);
    ssd1306_render_buffer(bf);
}

void clear()
{
    buffer_clear(bf);
    ssd1306_render_buffer(bf);
}

void set_position(uint8_t x, uint8_t y)
{
    pos_x = x * toshiba_font.char_w;
    pos_y = y * toshiba_font.char_h + 3;
}

}
