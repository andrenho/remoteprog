#ifndef LASTCALL_HH
#define LASTCALL_HH

#include <unordered_map>
#include <string>

namespace lastcall {

using Data = std::unordered_map<std::string, std::string>;

Data load();
void save(Data const& data);

}

#endif //LASTCALL_HH
