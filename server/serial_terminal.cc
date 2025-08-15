#include "serial_terminal.hh"

#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <sys/reboot.h>

#include "ui.hh"
#include "pigpio.h"

#define SERIAL_PIN 23

namespace serial_terminal {

void set_serial_and_reset()
{
    if (!get()) {
        bool ready_to_reboot = false;

        // update /boot/firmware/cmdline.txt
        std::string text;
        {
            std::stringstream buffer;
            std::ifstream f("/boot/firmware/cmdline.txt");
            buffer << f.rdbuf();
            text = "console=serial0,115200 " + buffer.str();
        }
        {
            std::ofstream f("/boot/firmware/cmdline.txt");
            if (f.is_open()) {
                f << text;
                ready_to_reboot = true;
            }
        }

        // reboot
        if (ready_to_reboot) {
            ui::clear();
            ui::set_position(0, 0);
            ui::print("Rebooting...");
            ui::beep_success();
            
            sync();
            reboot(LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, RB_AUTOBOOT, nullptr);
        }
    }
}

void init()
{
    gpioSetPullUpDown(SERIAL_PIN, PI_PUD_UP);
    gpioSetAlertFunc(SERIAL_PIN, [](int, int level, uint32_t) {
        if (level == 0) {
            gpioSetTimerFunc(0, 2000, []() {
                if (gpioRead(SERIAL_PIN) == 0)
                    set_serial_and_reset();
            });
        }
    });
}

bool get()
{
    std::stringstream buffer;
    std::ifstream f("/boot/firmware/cmdline.txt");
    buffer << f.rdbuf();
    std::string text = buffer.str();

    return text.find("serial0") != std::string::npos;
}

}
