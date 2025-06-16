#include <cstdlib>
#include <pigpio.h>
#include <unistd.h>

#include <stdexcept>

#define BEEP 16
#define RS 17
#define EM 27
#define D4 19
#define D5 5
#define D6 6
#define D7 13

namespace ui {

static void latch()
{
    gpioWrite(EM, 0);
    usleep(1000);
    gpioWrite(EM, 1);
    usleep(1000);
    gpioWrite(EM, 0);
    usleep(100);
}

static void write_4bit(uint8_t data)
{
    if (data & 0x1) {
        gpioWrite(D4, 1);
    } else {
        gpioWrite(D4, 0);
    }

    if (data & 0x2) {
        gpioWrite(D5, 1);
    } else {
        gpioWrite(D5, 0);
    }

    if (data & 0x4) {
        gpioWrite(D6, 1);
    } else {
        gpioWrite(D6, 0);
    }

    if (data & 0x8) {
        gpioWrite(D7, 1);
    } else {
        gpioWrite(D7, 0);
    }

    latch();
}

static void init_display()
{
    usleep(100000);
    gpioWrite(RS, 0);
    gpioWrite(EM, 0);

    write_4bit(0b0011);
    usleep(4500);
    write_4bit(0b0011);
    usleep(4500);
    write_4bit(0b0011);
    usleep(150);

    write_4bit(0b0010); // initial function set
    usleep(150);

    write_4bit(0b0010); // function set
    write_4bit(0b1000);

    write_4bit(0b0000); // display control
    write_4bit(0b1000);

    write_4bit(0b0000); // clear display
    write_4bit(0b0001);
    usleep(3000);

    write_4bit(0b0000); // entry mode set
    write_4bit(0b0110);

    write_4bit(0b0000); // display control
    write_4bit(0b1100);
}

static void write_byte(uint8_t data)
{
    write_4bit(data >> 4);
    write_4bit(data);
}

static void write_command(uint8_t data)
{
    gpioWrite(RS, 0);
    write_byte(data);
}

static void write_data(uint8_t data)
{
    gpioWrite(RS, 1);
    write_byte(data);
}

void init()
{
    if (gpioInitialise() < 0)
        throw std::runtime_error("Could not initialize pigpio.");

    gpioSetMode(BEEP, PI_OUTPUT);
    gpioSetMode(RS, PI_OUTPUT);
    gpioSetMode(EM, PI_OUTPUT);
    gpioSetMode(D4, PI_OUTPUT);
    gpioSetMode(D5, PI_OUTPUT);
    gpioSetMode(D6, PI_OUTPUT);
    gpioSetMode(D7, PI_OUTPUT);

    init_display();
}

void close()
{
    gpioTerminate();
}

void beep_success()
{
    gpioSetPWMfrequency(BEEP, 400);
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
    while (*str) {
        write_data(*str);
        str += 1;
    }
}

void clear()
{
    write_command(0x01);
}

void set_position(uint8_t x, uint8_t y)
{
    if (y == 0) {
        write_command(0x80 | x);
    }
    if (y == 1) {
        write_command(0x80 | 0x40 | x);
    }
}

}
