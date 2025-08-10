#ifndef LLCOMM_HH
#define LLCOMM_HH

#include "messages.pb.h"

namespace llcomm {

void init();
void close();

void reset(Request_Reset const& reset);

}

#endif //LLCOMM_HH
