#include "hmq/message_log.hpp"

#include <array>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <utility>

MessageLog::MessageLog(const std::filesystem::path &path) : path_(path) {}

void MessageLog::append(const std::string &content)
{
    if (content.size() > std::numeric_limits<std::uint32_t>::max())
    {
        throw std::length_error("Message exceeds maximum supported size");
    }

    const auto length = static_cast<std::uint32_t>(content.size());

    // encode length as little-endian 4 bytes
    std::array<char, 4> header{
        static_cast<char>(length & 0xFF),
        static_cast<char>((length >> 8) & 0xFF),
        static_cast<char>((length >> 16) & 0xFF),
        static_cast<char>((length >> 24) & 0xFF)};

    std::ofstream file(path_, std::ios::binary | std::ios::app);
    if (!file)
    {
        throw std::runtime_error("Failed to open message log file for appending");
    }

    file.write(header.data(), static_cast<std::streamsize>(header.size()));
    file.write(content.data(), static_cast<std::streamsize>(content.size()));

    if (!file)
    {
        throw std::runtime_error("Failed to write message to log file");
    }
}

std::vector<Message> MessageLog::read_all() const
{
    std::ifstream file(path_, std::ios::binary);
    if (!file)
    {
        throw std::runtime_error("Failed to open message log file for reading");
    }

    std::vector<Message> messages;
    std::size_t offset = 0;

    while (true)
    {
        std::array<unsigned char, 4> header{};
        file.read(reinterpret_cast<char *>(header.data()), static_cast<std::streamsize>(header.size()));

        const std::streamsize bytes_read = file.gcount();
        if (bytes_read == 0 && file.eof())
        {
            break; // End of file reached
        }
        if (bytes_read != static_cast<std::streamsize>(header.size()))
        {
            throw std::runtime_error("Failed to read message length from log file");
        }

        // decode length from little-endian 4 bytes
        const std::uint32_t length = static_cast<std::uint32_t>(header[0]) |
                                     (static_cast<std::uint32_t>(header[1]) << 8) |
                                     (static_cast<std::uint32_t>(header[2]) << 16) |
                                     (static_cast<std::uint32_t>(header[3]) << 24);

        std::string content(length, '\0');
        file.read(&content[0], static_cast<std::streamsize>(length));
        if (file.gcount() != static_cast<std::streamsize>(length))
        {
            throw std::runtime_error("Failed to read message content from log file");
        }

        messages.push_back({offset++, std::move(content)});
    }
    return messages;
}