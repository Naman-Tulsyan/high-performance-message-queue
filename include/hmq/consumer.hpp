#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <memory>

namespace hmq
{
    class Consumer
    {
    private:
        struct Impl;
        std::unique_ptr<Impl> impl_;

    public:
        Consumer(const std::string &host, std::uint16_t port);
        ~Consumer();

        Consumer(const Consumer &) = delete;            // Disable copy constructor (creates a new instance by copying an existing one)
        Consumer &operator=(const Consumer &) = delete; // Disable copy assignment operator (modifies an existing instance by copying another one)

        Consumer(Consumer &&) noexcept;            // Move constructor (creates a new instance by transferring ownership of resources from an existing one)
        Consumer &operator=(Consumer &&) noexcept; // Move assignment operator (modifies an existing instance by transferring ownership of resources from another one)

        // optional because the broker may not have a message at the requested offset, in which case it will return an empty response.
        std::optional<std::string> consume(std::uint64_t offset);
    };
}
