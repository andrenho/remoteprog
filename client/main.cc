#include "client.hh"

int main()
{
    client::connect("localhost");

    Request request;
    request.set_ack(true);

    Response response = client::send_request(request);

    if (response.response_case() == Response::kResult && response.result().success())
        printf("Success\n");
    else
        printf("Failure\n");
}