#include "hmq/tcp_client.hpp"

#include <array>
#include <cerrno>
#include <stdexcept>
#include <string>
#include <vector>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

namespace hmq
{
    namespace
    {
        bool read_exact(int fd, std::uint8_t *buffer, std::size_t size)
        {
            std::size_t received = 0;

            while (received < size)
            {
                const ssize_t n =
                    ::recv(fd, buffer + received, size - received, 0);

                if (n == 0)
                {
                    throw std::runtime_error("Server disconnected");
                }

                if (n < 0)
                {
                    if (errno == EINTR)
                    {
                        continue;
                    }
                    throw std::runtime_error("Failed to receive response");
                }

                received += static_cast<std::size_t>(n);
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

    } // namespace

    TcpClient::TcpClient(const std::string &host, std::uint16_t port)
    {
        addrinfo hints{};                // addrinfo is a structure that holds information about an address. It is used to specify the criteria for selecting the socket address structures returned by getaddrinfo.
        hints.ai_family = AF_UNSPEC;     // AF_UNSPEC allows for both IPv4 and IPv6 addresses to be returned by getaddrinfo.
        hints.ai_socktype = SOCK_STREAM; // SOCK_STREAM indicates that we want a stream socket (TCP) rather than a datagram socket (UDP).

        addrinfo *results = nullptr;
        const std::string service = std::to_string(port);

        // getaddrinfo is a function that performs DNS resolution and returns a linked list of addrinfo structures that match the specified criteria (hints) for the given host and service (port).
        // .c_str() is used to convert the std::string to a C-style string (const char*) because getaddrinfo expects C-style strings as input.
        const int status = ::getaddrinfo(host.c_str(), service.c_str(), &hints, &results);

        if (status != 0)
        {
            throw std::runtime_error(std::string("Address lookup failed: ") + ::gai_strerror(status));
        }

        for (addrinfo *result = results; result != nullptr; result = result->ai_next)
        {
            const int fd = ::socket(result->ai_family, result->ai_socktype, result->ai_protocol);

            if (fd < 0)
            {
                continue;
            }
            if (::connect(fd, result->ai_addr, result->ai_addrlen) == 0)
            {
                socket_fd_ = fd;
                break;
            }

            ::close(fd);
        }

        ::freeaddrinfo(results);

        if (socket_fd_ < 0)
        {
            throw std::runtime_error("Failed to connect to HMQ server");
        }
    }

    TcpClient::~TcpClient()
    {
        if (socket_fd_ >= 0)
        {
            ::close(socket_fd_);
        }
    }

    void TcpClient::send_all(const std::vector<std::uint8_t> &bytes)
    {
        std::size_t sent = 0;

        while (sent < bytes.size())
        {
            // MSG_NOSIGNAL is a flag that can be passed to the send function to prevent the generation of a SIGPIPE signal when sending data to a closed socket. If MSG_NOSIGNAL is not available, we can use 0 as the flags value.
#ifdef MSG_NOSIGNAL
            constexpr int flags = MSG_NOSIGNAL;
#else
            constexpr int flags = 0;
#endif

            const ssize_t n = ::send(socket_fd_, bytes.data() + sent, bytes.size() - sent, flags);

            if (n < 0)
            {
                if (errno == EINTR)
                {
                    continue;
                }
                throw std::runtime_error("Failed to send request");
            }
            if (n == 0)
            {
                throw std::runtime_error("Connection closed during send");
            }

            sent += static_cast<std::size_t>(n);
        }
    }

    std::vector<std::uint8_t> TcpClient::receive_frame()
    {
        std::array<std::uint8_t, protocol::HEADER_SIZE> header{};

        read_exact(socket_fd_, header.data(), header.size());

        // Reject invalid framing before allocating the payload.
        for (std::size_t i = 0; i < protocol::MAGIC.size(); ++i)
        {
            if (header[i] != protocol::MAGIC[i])
            {
                throw std::runtime_error("Invalid response magic");
            }
        }

        const std::uint16_t version =
            (static_cast<std::uint16_t>(header[4]) << 8) |
            static_cast<std::uint16_t>(header[5]);

        if (version != protocol::VERSION)
        {
            throw std::runtime_error("Unsupported response version");
        }

        const std::uint32_t payload_size = decode_u32_be(header.data() + 12);

        if (payload_size > protocol::MAX_PAYLOAD_SIZE)
        {
            throw std::runtime_error("Response payload too large");
        }

        std::vector<std::uint8_t> bytes(header.begin(), header.end());

        bytes.resize(protocol::HEADER_SIZE + payload_size);

        if (payload_size > 0)
        {
            read_exact(socket_fd_, bytes.data() + protocol::HEADER_SIZE, payload_size);
        }

        return bytes;
    }

    protocol::Frame TcpClient::request(const protocol::Frame &frame)
    {
        const auto bytes = protocol::encode_frame(frame);
        send_all(bytes);

        const auto response_bytes = receive_frame();
        auto response = protocol::decode_frame(response_bytes);

        if (response.request_id != frame.request_id)
        {
            throw std::runtime_error("Response request ID mismatch");
        }

        return response;
    }

} // namespace hmq
