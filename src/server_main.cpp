#include "hmq/tcp_server.hpp"

#include <cstdint>
#include <exception>
#include <iostream>

int main()
{
    try
    {
        hmq::TcpServer server(9090);
        server.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "HMQ server error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
