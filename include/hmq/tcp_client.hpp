#pragma once

#include "hmq/protocol.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace hmq
{
    class TcpClient
    {
    private:
        int socket_fd_ = -1;

        void send_all(const std::vector<std::uint8_t> &bytes);
        std::vector<std::uint8_t> receive_frame();

    public:
        TcpClient(const std::string &host, std::uint16_t port); // constructor that takes a host and port, and establishes a TCP connection to the server.
        ~TcpClient();                                           // destructor that closes the socket file descriptor if it is valid (greater than or equal to 0).

        // = delete the copy constructor and copy assignment operator to prevent copying of TcpClient instances, as they manage a socket file descriptor that should not be duplicated.
        // RAII (Resource Acquisition Is Initialization) principle is followed here, where the resource (socket file descriptor) is acquired in the constructor and released in the destructor, ensuring proper resource management.
        TcpClient(const TcpClient &) = delete;            // copy constructor
        TcpClient &operator=(const TcpClient &) = delete; // copy assignment operator

        protocol::Frame request(const protocol::Frame &frame);
    };
} // namespace hmq
