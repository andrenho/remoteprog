#include <cstdlib>
#include <cstdio>
#include <pigpio.h>
#include <unistd.h>

#include <stdexcept>

#define BEEP 16
#define RS 17
#define EM 27
#define D4 0
#define D5 5
#define D6 6
#define D7 7

namespace ui {

static void latch() {
  gpioWrite(EM, 0);
  usleep(1000);
  gpioWrite(EM, 1);
  usleep(1000);
  gpioWrite(EM, 0);
  usleep(100);
}

static void write4Bit(uint8_t data) {
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

static void init_display() {
    usleep(50000);
    gpioWrite(RS, 0);
    gpioWrite(EM, 0);

    write4Bit(0x03);
    usleep(4500);
    write4Bit(0x03);
    usleep(4500);
    write4Bit(0x03);
    usleep(150);
    write4Bit(0x02);

    write4Bit(0x02);
    write4Bit(0x0C);
    write4Bit(0x00);
    write4Bit(0x08);
    write4Bit(0x00);
    write4Bit(0x01);
    write4Bit(0x00);
    write4Bit(0x06);

    write4Bit(0x00);
    write4Bit(0x0F);
}

static void writeByte(uint8_t data) {
  write4Bit(data >> 4);
  write4Bit(data);
}

static void writeCommand(uint8_t data) {
  gpioWrite(RS, 0);
  writeByte(data);
}

static void writeData(uint8_t data) {
  gpioWrite(RS, 1);
  writeByte(data);
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


void print(const char *str) {
  while (*str) {
    writeData(*str);
    str += 1;
  }
}

void clear(){
   writeCommand(0x01);
}

void setPosition(uint8_t x, uint8_t y) {
  if (y == 0) {
    writeCommand(0x80 | x);
  }
  if (y == 1) {
    writeCommand(0x80 | 0x40 | x);
  }
}

}

int main()
{
    ui::init();
    ui::beep_success();
    ui::clear();
    ui::print("Hello");
    ui::beep_success();

    ui::close();
}
