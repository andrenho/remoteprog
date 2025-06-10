#include "server.hh"

int main()
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    server::listen();
}