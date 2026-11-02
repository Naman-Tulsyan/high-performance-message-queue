
#pragma once

#include "hmq/protocol.hpp"

#include <array>
#include <cerrno>
#include <cstdint>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace hmq::test
{
    using Bytes = std::vector<std::uint8_t>;

    inline bool read_exact(int fd, std::uint8_t *data, std::size_t size)
    {
        std::size_t received = 0;

        while (received < size)
        {
            const auto n = recv(fd, data + received, size - received, 0);

            if (n == 0)
                return false;

            if (n < 0)
            {
                if (errno == EINTR)
                    continue;

                return false;
            }

            received += static_cast<std::size_t>(n);
        }

        return true;
    }

    inline void send_all(int fd, const Bytes &bytes)
    {
        std::size_t sent = 0;

        while (sent < bytes.size())
        {
            const auto n = send(fd, bytes.data() + sent, bytes.size() - sent, 0);

            if (n < 0 && errno == EINTR)
                continue;

            if (n <= 0)
                throw std::runtime_error("Mock server send failed");

            sent += static_cast<std::size_t>(n);
        }
    }

    class MockServer
    {
    public:
        explicit MockServer(Bytes response)
        {
            listener_ = socket(AF_INET, SOCK_STREAM, 0);

            if (listener_ < 0)
                throw std::runtime_error("socket failed");

            sockaddr_in address{};
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            address.sin_port = 0;

            if (bind(listener_, reinterpret_cast<sockaddr *>(&address), sizeof(address)) < 0)
            {
                close(listener_);
                throw std::runtime_error("bind failed");
            }

            if (listen(listener_, 1) < 0)
            {
                close(listener_);
                throw std::runtime_error("listen failed");
            }

            socklen_t length = sizeof(address);

            if (getsockname(listener_, reinterpret_cast<sockaddr *>(&address), &length) < 0)
            {
                close(listener_);
                throw std::runtime_error("getsockname failed");
            }

            port_ = ntohs(address.sin_port);

            worker_ = std::thread(
                [this, response = std::move(response)]()
                {
                    const int client = accept(listener_, nullptr, nullptr);

                    if (client < 0)
                        return;

                    try
                    {
                        std::array<std::uint8_t, protocol::HEADER_SIZE> header{};

                        if (!read_exact(client, header.data(), header.size()))
                        {
                            close(client);
                            return;
                        }

                        // Payload length is stored in bytes 12–15.
                        const std::uint32_t size =
                            (std::uint32_t(header[12]) << 24) |
                            (std::uint32_t(header[13]) << 16) |
                            (std::uint32_t(header[14]) << 8) |
                            std::uint32_t(header[15]);

                        if (size > protocol::MAX_PAYLOAD_SIZE)
                        {
                            close(client);
                            return;
                        }

                        Bytes payload(size);

                        if (size > 0 && !read_exact(client, payload.data(), payload.size()))
                        {
                            close(client);
                            return;
                        }

                        send_all(client, response);
                    }
                    catch (...)
                    {
                        // The test checks the client-side result.
                    }

                    shutdown(client, SHUT_RDWR);
                    close(client);
                });
        }

        ~MockServer()
        {
            if (worker_.joinable())
                worker_.join();

            if (listener_ >= 0)
                close(listener_);
        }

        MockServer(const MockServer &) = delete;
        MockServer &operator=(const MockServer &) = delete;

        std::uint16_t port() const
        {
            return port_;
        }

    private:
        int listener_ = -1;
        std::uint16_t port_ = 0;
        std::thread worker_;
    };
}
