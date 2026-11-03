
#pragma once

#include "hmq/message_log.hpp"
#include "hmq/protocol.hpp"

#include <filesystem>
#include <vector>

namespace hmq
{
    class Broker
    {
    private:
        MessageLog log_;
        std::vector<Message> messages_;

    public:
        explicit Broker(const std::filesystem::path &log_path);

        protocol::Frame handle(const protocol::Frame &request);
    };
}
