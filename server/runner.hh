#ifndef RUNNER_HH
#define RUNNER_HH

#include <string>

namespace runner {

void execute(int response_fd, std::string const& command);

}

#endif //RUNNER_HH
