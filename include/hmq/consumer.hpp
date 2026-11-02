#pragma once

#include "hmq/tcp_client.hpp"

#include <cstdint>
#include <optional>
#include <string>

namespace hmq
{
    class Consumer
    {
    private:
        TcpClient client_;
        std::uint32_t next_request_id_ = 1;

    public:
        Consumer(const std::string &host, std::uint16_t port);

        // optional because the broker may not have a message at the requested offset, in which case it will return an empty response.
        std::optional<std::string> consume(std::uint64_t offset);
    };
}