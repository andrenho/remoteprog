#ifndef RUNNER_HH
#define RUNNER_HH

#include <string>
#include <vector>

namespace runner {

bool execute(int response_fd, std::vector<std::string> const& command, bool debug);

}

#endif //RUNNER_HH
