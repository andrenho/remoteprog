#include "server.hh"

#include <cstring>
#include <cstdlib>
#include <ctime>
#include <csignal>
#include <sys/types.h>

#include "llcomm.hh"
#include "ui.hh"

static int update_network_tries = 0;
static timer_t timer;

static void update_network_signal(int sig)
{
    (void) sig;

    auto ip = server::local_ip();

    ui::set_position(0, 0);
    if (ip) {
        ui::print(ip->c_str());
    } else {
        ui::print("No network");
        ++update_network_tries;
    }

    if (ip || update_network_tries > 5)
        timer_delete(timer);
}

static void update_network_timer()   // create timer to update IP address on display
{
    signal(SIGALRM, update_network_signal);

    sigevent sev;
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGALRM;
    sev.sigev_value.sival_ptr = &timer;

    timer_create(CLOCK_REALTIME, &sev, &timer);

    itimerspec ts;
    ts.it_value.tv_sec = 1;
    ts.it_value.tv_nsec = 0;
    ts.it_interval.tv_sec = 5;
    ts.it_interval.tv_nsec = 0;

    timer_settime(timer, 0, &ts, NULL);
}

int main(int argc, char* argv[])
{
    srand(time(nullptr));

    bool debug_mode = false;
    if (argc == 2 && strcmp(argv[1], "-d") == 0)
        debug_mode = true;

    llcomm::init();
    ui::init();
    ui::beep_success();

    update_network_timer();

    server::listen(debug_mode);

    llcomm::close();  // TODO - catch SIGINT
}
