#pragma once

#include <cstdint>

#include "hmq/broker.hpp"

namespace hmq
{
    class TcpServer
    {
    private:
        std::uint16_t port_;
        Broker broker_;

    public:
        TcpServer(std::uint16_t port, const std::filesystem::path &log_path);
        void run();
    };
} // namespace hmq
