#include "hmq/tcp_client.hpp"

#include <exception>
#include <iostream>

int main()
{
    try
    {
        hmq::TcpClient client("127.0.0.1", 9090);

        hmq::protocol::Frame request;
        request.type = hmq::protocol::MessageType::PING;
        request.request_id = 42;

        const auto response = client.request(request);

        if (response.type != hmq::protocol::MessageType::PING_RESPONSE)
        {
            std::cerr << "Unexpected response type\n";
            return 1;
        }

        std::cout << "PING successful; request ID: " << response.request_id << '\n';
    }
    catch (const std::exception &e)
    {
        std::cerr << "Client error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
