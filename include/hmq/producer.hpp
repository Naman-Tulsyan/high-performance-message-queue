#pragma once

#include <cstdint>
#include <string>
#include <memory>

namespace hmq
{
    class Producer
    {
    private:
        struct Impl;
        std::unique_ptr<Impl> impl_;

    public:
        Producer(const std::string &host, std::uint16_t port);
        ~Producer();

        Producer(const Producer &) = delete;            // Disable copy constructor (creates a new instance by copying an existing one)
        Producer &operator=(const Producer &) = delete; // Disable copy assignment operator (modifies an existing instance by copying another one)

        Producer(Producer &&) noexcept;            // Move constructor (creates a new instance by transferring ownership of resources from an existing one)
        Producer &operator=(Producer &&) noexcept; // Move assignment operator (modifies an existing instance by transferring ownership of resources from another one)

        std::uint64_t publish(const std::string &content);
    };
}
