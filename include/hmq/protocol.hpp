#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace hmq::protocol
{

    constexpr std::size_t HEADER_SIZE = 16;
    constexpr std::uint32_t MAX_PAYLOAD_SIZE = 1024 * 1024;

    constexpr std::array<std::uint8_t, 4> MAGIC = {'H', 'M', 'Q', '1'};
    constexpr std::uint16_t VERSION = 1;

    enum class MessageType : std::uint16_t
    {
        PING = 0x0001,
        PING_RESPONSE = 0x8001,
        ERROR = 0xFFFF,
    };

    struct Frame
    {
        std::uint16_t version = VERSION;
        MessageType type = MessageType::PING;
        std::uint32_t request_id = 0;
        std::vector<std::uint8_t> payload;
    };

    // Encodes a frame into its wire representation.
    // Throws std::invalid_argument if the payload exceeds the limit.
    std::vector<std::uint8_t> encode_frame(const Frame &frame);

    // Decodes one complete frame.
    // Throws std::invalid_argument for malformed headers or invalid lengths.
    // Throws std::runtime_error if the input is incomplete.
    Frame decode_frame(const std::vector<std::uint8_t> &bytes);

} // namespace hmq::protocol