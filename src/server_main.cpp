#include "hmq/tcp_server.hpp"

#include <cstdint>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

int main(int argc, char *argv[])
{
    try
    {
        const auto port = (argc >= 2) ? static_cast<std::uint16_t>(std::stoi(argv[1])) : 9090UL;

        if (port == 0 || port > 65535)
        {
            throw std::runtime_error("Invalid port number");
        }

        hmq::TcpServer server(static_cast<std::uint16_t>(port));
        server.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "HMQ server error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
