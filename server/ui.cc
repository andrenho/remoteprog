#include <cstdlib>
#include <pigpio.h>
#include <unistd.h>

#include <stdexcept>
extern "C" {
#include "ssd1306/ssd1306.h"
}

#define BEEP 16

namespace ui {

static void init_display()
{
    SSD1306_Init(SSD1306_ADDR);
}

void init()
{
    gpioSetMode(BEEP, PI_OUTPUT);
    // TODO - init pins

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
    SSD1306_DrawString(str);
}

void clear()
{
    SSD1306_Clear();
}

void set_position(uint8_t x, uint8_t y)
{
    if (y == 0)
        SSD1306_SetPosition(7, 1);
    // TODO
}

}
