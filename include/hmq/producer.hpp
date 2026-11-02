#pragma once

#include "hmq/tcp_client.hpp"

#include <cstdint>
#include <string>

namespace hmq
{
    class Producer
    {
    private:
        TcpClient client_;
        std::uint32_t next_request_id_ = 1;

    public:
        Producer(const std::string &host, std::uint16_t port);

        std::uint64_t publish(const std::string &content);
    };
}