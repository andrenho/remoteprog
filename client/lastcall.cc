#include "lastcall.hh"

#include <fstream>
#include <sys/stat.h>

namespace lastcall {

Data load()
{
    Data data;
    std::string home = getenv("HOME");
    mkdir((home + "/.config").c_str(), 0755);
    std::ifstream f(home + "/.config/remoteprog.last");
    if (f.is_open()) {
        std::string line;
        while (std::getline(f, line)) {
            int eq = line.find('=');
            if (eq == std::string::npos)
                return {};
            data[line.substr(0, eq - 1)] = line.substr(eq + 1);
        }
        f.close();
    }
}

void save(Data const& data)
{
    std::string home = getenv("HOME");
    mkdir((home + "/.config").c_str(), 0755);
    std::ofstream f(home + "/.config/remoteprog.last");
    if (f.is_open()) {
        for (auto const& kv: data) {
            f << kv.first << "=" << kv.second << "\n";
        }
    }
}

}
