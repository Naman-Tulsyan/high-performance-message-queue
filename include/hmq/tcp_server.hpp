#pragma once

#include <cstdint>

namespace hmq
{
    class TcpServer
    {
    private:
        std::uint16_t port_;

    public:
        TcpServer(std::uint16_t port);
        void run();
    };
} // namespace hmq
