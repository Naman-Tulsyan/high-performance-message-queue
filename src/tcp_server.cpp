#include "hmq/tcp_server.hpp"
#include "hmq/protocol.hpp"

#include <array>
#include <cerrno>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <arpa/inet.h>  // converts IP addresses between binary and text form
#include <netinet/in.h> // define internet address family
#include <sys/socket.h> // provides socket functions and structures and API for socket programming
#include <unistd.h>

namespace hmq
{
    namespace
    {
        constexpr std::size_t HEADER_SIZE = protocol::HEADER_SIZE;
        constexpr std::uint32_t MAX_PAYLOAD = protocol::MAX_PAYLOAD_SIZE;

        // Reads exactly `size` bytes from the socket with file descriptor `fd` into `buffer`.
        bool read_exact(int fd, std::uint8_t *buffer, std::size_t size)
        {
            std::size_t received = 0;

            while (received < size)
            {
                // recv returns the number of bytes received, or -1 on error, or 0 if the connection is closed or no data available.
                // so this is signed size_t
                const ssize_t n = ::recv(fd, buffer + received, size - received, 0);

                if (n == 0)
                {
                    // Peer (client) disconnected, possibly halfway through a frame.
                    return false;
                }
                if (n < 0)
                {
                    if (errno == EINTR) // EINTR indicates that the call was interrupted by a signal before any data was read; in this case, we can continue reading.
                    {
                        continue;
                    }
                    return false;
                }

                received += static_cast<std::size_t>(n);
            }
            return true;
        }

        bool write_all(int fd, const std::uint8_t *buffer, std::size_t size)
        {
            std::size_t sent = 0;

            while (sent < size)
            {
                // send returns the number of bytes sent, or -1 on error and 0 if the connection is closed.
                const ssize_t n = ::send(fd, buffer + sent, size - sent, 0);

                if (n < 0)
                {
                    if (errno == EINTR)
                    {
                        continue;
                    }
                    return false;
                }
                if (n == 0)
                {
                    return false;
                }

                sent += static_cast<std::size_t>(n);
            }

            return true;
        }

        std::uint32_t decode_u32_be(const std::uint8_t *bytes)
        {
            return (static_cast<std::uint32_t>(bytes[0]) << 24) |
                   (static_cast<std::uint32_t>(bytes[1]) << 16) |
                   (static_cast<std::uint32_t>(bytes[2]) << 8) |
                   static_cast<std::uint32_t>(bytes[3]);
        }

        void handle_client(int client_fd)
        {
            while (true)
            {
                std::array<std::uint8_t, HEADER_SIZE> header{};

                if (!read_exact(client_fd, header.data(), header.size()))
                {
                    return;
                }

                // Read the advertised payload length only after receiving
                // the complete fixed-size header.
                const std::uint32_t payload_size = decode_u32_be(header.data() + 12);

                if (payload_size > MAX_PAYLOAD)
                {
                    std::cerr << "Client sent oversized payload\n";
                    return;
                }

                std::vector<std::uint8_t> bytes(header.begin(), header.end());
                bytes.resize(HEADER_SIZE + payload_size);

                if (payload_size > 0 && !read_exact(client_fd, bytes.data() + HEADER_SIZE, payload_size))
                {
                    return;
                }

                try
                {
                    const protocol::Frame request = protocol::decode_frame(bytes);

                    if (request.type != protocol::MessageType::PING)
                    {
                        protocol::Frame error;
                        error.type = protocol::MessageType::ERROR;
                        error.request_id = request.request_id;
                        error.payload = {'U', 'n', 's', 'u', 'p', 'p', 'o', 'r', 't', 'e', 'd', ' ', 'r', 'e', 'q', 'u', 'e', 's', 't'};

                        const auto response = protocol::encode_frame(error);

                        if (!write_all(client_fd, response.data(), response.size()))
                        {
                            return;
                        }
                        continue;
                    }

                    protocol::Frame response;
                    response.type = protocol::MessageType::PING_RESPONSE;
                    response.request_id = request.request_id;

                    const auto encoded = protocol::encode_frame(response);

                    if (!write_all(client_fd, encoded.data(), encoded.size()))
                    {
                        return;
                    }
                }
                catch (const std::exception &e)
                {
                    std::cerr << "Invalid client frame: " << e.what() << '\n';
                    return;
                }
            }
        }

    } // namespace

    TcpServer::TcpServer(std::uint16_t port) : port_(port) {}

    void TcpServer::run()
    {
        // Create a TCP socket using the IPv4 address family (AF_INET) and the stream socket type (SOCK_STREAM).
        const int server_fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0)
        {
            throw std::runtime_error("Failed to create TCP socket");
        }

        int reuse = 1;
        if (::setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
        {
            ::close(server_fd);
            throw std::runtime_error("Failed to configure socket");
        }

        sockaddr_in address{}; // sockaddr_in is a structure that holds an internet address. It is used to specify the address and port for the socket.
        address.sin_family = AF_INET;
        address.sin_port = htons(port_);                  // htons converts the port number from host byte order to network byte order (big-endian). host to network short
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // htonl converts the IP address from host byte order to network byte order (big-endian). host to network long

        if (::bind(server_fd, reinterpret_cast<sockaddr *>(&address), sizeof(address)) < 0)
        {
            ::close(server_fd);
            throw std::runtime_error("Failed to bind TCP port");
        }

        if (::listen(server_fd, SOMAXCONN) < 0) // SOMAXCONN is a constant that specifies the maximum number of pending connections that can be queued for acceptance. It is defined in <sys/socket.h> and is typically set to a system-defined value.
        {
            ::close(server_fd);
            throw std::runtime_error("Failed to listen on TCP socket");
        }

        std::cout << "HMQ server listening on 127.0.0.1:" << port_ << '\n';

        while (true)
        {
            // accept is blocking as it wait for a client to connect. It returns a new socket file descriptor for the accepted connection.
            const int client_fd = ::accept(server_fd, nullptr, nullptr);

            if (client_fd < 0)
            {
                if (errno == EINTR)
                {
                    continue;
                }
                ::close(server_fd);
                throw std::runtime_error("Failed to accept client");
            }

            handle_client(client_fd);
            ::close(client_fd);
        }
    }
} // namespace hmq