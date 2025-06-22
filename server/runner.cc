#include "runner.hh"

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/select.h>

#include <string>
#include <vector>
using namespace std::string_literals;

#include "messages.pb.h"
#include "../protobuf/comm.hh"

namespace runner {

bool execute(int response_fd, std::vector<std::string> const& command, bool debug)
{
    printf("Running command: ");
    for (auto const& cmd: command)
        if (cmd.find(' ') != std::string::npos)
            printf("\"%s\" ", cmd.c_str());
        else
            printf("%s ", cmd.c_str());
    printf("\n");

    char** args = (char **) calloc(sizeof(char *), command.size() + 1);
    for (size_t i = 0; i < command.size(); ++i) {
        args[i] = (char *) calloc(1, command.at(i).size() + 1);
        strcpy(args[i], command.at(i).c_str());
    }

    int stdout_pipe[2], stderr_pipe[2];
    if (pipe(stdout_pipe) == -1 || pipe(stderr_pipe) == -1)
        throw std::runtime_error("pipe(): "s + strerror(errno));

    pid_t pid = fork();
    if (pid == -1)
        throw std::runtime_error("fork(): "s + strerror(errno));

    if (pid == 0) { // child process
        // close read end
        close(stdout_pipe[0]);
        close(stderr_pipe[0]);

        // redirect stdout and stderr to pipes
        dup2(stdout_pipe[1], STDOUT_FILENO);
        dup2(stderr_pipe[1], STDERR_FILENO);
        close(stdout_pipe[1]);
        close(stderr_pipe[1]);

        // execute command
        execvp(args[0], args);
        throw std::runtime_error("execpv(): "s + strerror(errno));
    }

    for (size_t i = 0; i < command.size(); ++i)
        free(args[i]);
    free(args);

    // Parent process
    close(stdout_pipe[1]); // Close write end
    close(stderr_pipe[1]);

    // Make pipes non-blocking
    fcntl(stdout_pipe[0], F_SETFL, O_NONBLOCK);
    fcntl(stderr_pipe[0], F_SETFL, O_NONBLOCK);

    int max_fd = (stdout_pipe[0] > stderr_pipe[0]) ? stdout_pipe[0] : stderr_pipe[0];
    fd_set readfds;
    std::string stdout_buffer, stderr_buffer;
    int status;

    auto process_pipe = [&](int pipe, std::string& buffer) {
        char buf[1024];
        ssize_t bytes = read(pipe, buf, sizeof(buf) - 1);
        if (bytes > 0) {
            buf[bytes] = '\0';
            buffer += buf;
        }
    };

    auto process_buffer = [](std::string& buffer) -> std::string {
        size_t p = buffer.rfind('\n');
        if (p == std::string::npos)
            return "";
        std::string to_send = buffer.substr(0, p);
        buffer = buffer.substr(p + 1);
        return to_send;
    };

    while (true) {
        FD_ZERO(&readfds);
        FD_SET(stdout_pipe[0], &readfds);
        FD_SET(stderr_pipe[0], &readfds);

        // Wait for data with timeout
        timeval timeout = {0, 100000}; // 100ms timeout
        int r = select(max_fd + 1, &readfds, nullptr, nullptr, &timeout);

        // read from pipes
        if (r > 0) {
            if (FD_ISSET(stdout_pipe[0], &readfds))
                process_pipe(stdout_pipe[0], stdout_buffer);
            if (FD_ISSET(stderr_pipe[0], &readfds))
                process_pipe(stderr_pipe[0], stderr_buffer);
        }

        // send partial responses
        std::string stdout_to_send = process_buffer(stdout_buffer);
        std::string stderr_to_send = process_buffer(stderr_buffer);
        if (!stdout_to_send.empty() || !stderr_to_send.empty()) {
            auto result = new Response_Result;
            result->set_result_code(Response_ResultCode_ONGOING);
            result->set_messages(stdout_to_send);
            result->set_errors(stderr_to_send);
            Response response;
            response.set_allocated_result(result);
            send_message(response_fd, response, debug);
        }

        // Check if child process is still running
        if (waitpid(pid, &status, WNOHANG) != 0) {
            break; // Child has exited
        }
    }

    // Read any remaining data
    process_pipe(stdout_pipe[0], stdout_buffer);
    process_pipe(stderr_pipe[0], stderr_buffer);

    close(stdout_pipe[0]);
    close(stderr_pipe[0]);

    // Wait for child to complete
    waitpid(pid, &status, 0);

    // send final message
    auto result = new Response_Result;
    result->set_result_code(status == 0 ? Response_ResultCode_SUCCESS : Response_ResultCode_FAILURE);
    result->set_messages(stdout_buffer);
    result->set_errors(stderr_buffer);
    Response response;
    response.set_allocated_result(result);
    send_message(response_fd, response, debug);

    return status == 0;
}

}
