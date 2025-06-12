#include "runner.hh"

#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>

#include <string>
using namespace std::string_literals;

namespace runner {

void execute(int response_fd, char* args[])
{
    int stdout_pipe[2], stderr_pipe[2];
    if (pipe(stdout_pipe) == -1 || pipe(stderr_pipe) == -1)
        throw std::runtime_error("pipe(): "s + strerror(errno));

    pid_t pid = fork();
    if (pid == -1)
        throw std::runtime_error("fork(): "s + strerror(errno));

    if (pid == 0) { // child process
        close(stdout_pipe[0]);
        close(stderr_pipe[0]);
        dup2(stdout_pipe[1], STDOUT_FILENO);
        dup2(stderr_pipe[1], STDERR_FILENO);
        close(stdout_pipe[1]);
        close(stderr_pipe[1]);
        execvp(args[0], &args[1]);
        throw std::runtime_error("execpv(): "s + strerror(errno));
    } else {
        // parent process

    }
}

}
