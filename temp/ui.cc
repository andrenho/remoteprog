#include <cstdlib>
#include <cstdio>
#include <pigpio.h>
#include <unistd.h>

#include <stdexcept>

#define BEEP 16

class UserInterface {
public:
    UserInterface()
    {
        if (gpioInitialise() < 0)
            throw std::runtime_error("Could not initialize pigpio.");

        // beeper
        gpioSetMode(BEEP, PI_OUTPUT);
    }

    ~UserInterface()
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
        gpioSetPWMfrequency(BEEP, 200);
        gpioPWM(BEEP, 128);
        usleep(500000);
        gpioPWM(BEEP, 0);
        usleep(200000);
        gpioPWM(BEEP, 128);
        usleep(500000);
        gpioPWM(BEEP, 0);
    }
};

int main()
{
    UserInterface ui;
    ui.beep_success();
}
