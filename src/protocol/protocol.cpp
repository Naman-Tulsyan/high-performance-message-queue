#include "hmq/protocol.hpp"
#include "hmq/byte_order.hpp"

#include <stdexcept>

namespace hmq::protocol
{
    std::vector<std::uint8_t> encode_frame(const Frame &frame)
    {
        if (frame.version != VERSION)
        {
            throw std::invalid_argument("Unsupported protocol version");
        }

        if (frame.payload.size() > MAX_PAYLOAD_SIZE)
        {
            throw std::invalid_argument("Payload exceeds maximum size");
        }

        std::vector<std::uint8_t> bytes(HEADER_SIZE + frame.payload.size());

        for (std::size_t i = 0; i < MAGIC.size(); ++i)
        {
            bytes[i] = MAGIC[i];
        }

        hmq::byte_order::write_u16_be(bytes.data() + 4, frame.version);
        hmq::byte_order::write_u16_be(bytes.data() + 6, static_cast<std::uint16_t>(frame.type));
        hmq::byte_order::write_u32_be(bytes.data() + 8, frame.request_id);
        hmq::byte_order::write_u32_be(bytes.data() + 12, static_cast<std::uint32_t>(frame.payload.size()));

        for (std::size_t i = 0; i < frame.payload.size(); ++i)
        {
            bytes[HEADER_SIZE + i] = frame.payload[i];
        }

        return bytes;
    }

    Frame decode_frame(const std::vector<std::uint8_t> &bytes)
    {
        if (bytes.size() < HEADER_SIZE)
        {
            throw std::runtime_error("Incomplete protocol header");
        }

        for (std::size_t i = 0; i < MAGIC.size(); ++i)
        {
            if (bytes[i] != MAGIC[i])
            {
                throw std::invalid_argument("Invalid protocol magic");
            }
        }

        const std::uint16_t version = hmq::byte_order::read_u16_be(bytes.data() + 4);

        if (version != VERSION)
        {
            throw std::invalid_argument("Unsupported protocol version");
        }

        const std::uint32_t payload_size = hmq::byte_order::read_u32_be(bytes.data() + 12);

        if (payload_size > MAX_PAYLOAD_SIZE)
        {
            throw std::invalid_argument("Payload exceeds maximum size");
        }

        if (bytes.size() != HEADER_SIZE + payload_size)
        {
            throw std::runtime_error("Frame size does not match payload length");
        }

        Frame frame;
        frame.version = version;
        frame.type = static_cast<MessageType>(hmq::byte_order::read_u16_be(bytes.data() + 6));
        frame.request_id = hmq::byte_order::read_u32_be(bytes.data() + 8);

        frame.payload.assign(
            bytes.begin() + static_cast<std::ptrdiff_t>(HEADER_SIZE),
            bytes.end());

        return frame;
    }

    std::vector<std::uint8_t> encode_offset(std::uint64_t offset)
    {
        std::vector<std::uint8_t> payload(8);
        hmq::byte_order::write_u64_be(payload.data(), offset);
        return payload;
    }

    std::uint64_t decode_offset(const std::vector<std::uint8_t> &payload)
    {
        if (payload.size() != 8)
        {
            throw std::invalid_argument("Offset payload must contain exactly 8 bytes");
        }
        return hmq::byte_order::read_u64_be(payload.data());
    }

    std::vector<std::uint8_t> encode_error(ErrorCode code, const std::string &description)
    {
        if (description.size() > MAX_PAYLOAD_SIZE - 2)
        {
            throw std::length_error("Error description exceeds maximum payload size");
        }

        std::vector<std::uint8_t> payload(2 + description.size());

        hmq::byte_order::write_u16_be(payload.data(), static_cast<std::uint16_t>(code));

        std::copy(description.begin(), description.end(), payload.begin() + 2);

        return payload;
    }

    ErrorCode decode_error_code(const std::vector<std::uint8_t> &payload)
    {
        if (payload.size() < 2)
        {
            throw std::invalid_argument("Error payload must contain at least 2 bytes");
        }
        return static_cast<ErrorCode>(hmq::byte_order::read_u16_be(payload.data()));
    }

} // namespace hmq::protocol