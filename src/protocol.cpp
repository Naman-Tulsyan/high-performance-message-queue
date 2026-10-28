#include "hmq/protocol.hpp"

#include <stdexcept>

namespace hmq::protocol
{
    namespace
    {

        void write_u16_be(
            std::vector<std::uint8_t> &out,
            std::size_t offset,
            std::uint16_t value)
        {
            out[offset] = static_cast<std::uint8_t>((value >> 8) & 0xFF);
            out[offset + 1] = static_cast<std::uint8_t>(value & 0xFF);
        }

        void write_u32_be(
            std::vector<std::uint8_t> &out,
            std::size_t offset,
            std::uint32_t value)
        {
            out[offset] = static_cast<std::uint8_t>((value >> 24) & 0xFF);
            out[offset + 1] = static_cast<std::uint8_t>((value >> 16) & 0xFF);
            out[offset + 2] = static_cast<std::uint8_t>((value >> 8) & 0xFF);
            out[offset + 3] = static_cast<std::uint8_t>(value & 0xFF);
        }

        std::uint16_t read_u16_be(
            const std::vector<std::uint8_t> &bytes,
            std::size_t offset)
        {
            return static_cast<std::uint16_t>(
                (static_cast<std::uint16_t>(bytes[offset]) << 8) |
                static_cast<std::uint16_t>(bytes[offset + 1]));
        }

        std::uint32_t read_u32_be(
            const std::vector<std::uint8_t> &bytes,
            std::size_t offset)
        {
            return (static_cast<std::uint32_t>(bytes[offset]) << 24) |
                   (static_cast<std::uint32_t>(bytes[offset + 1]) << 16) |
                   (static_cast<std::uint32_t>(bytes[offset + 2]) << 8) |
                   static_cast<std::uint32_t>(bytes[offset + 3]);
        }

    } // namespace

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

        write_u16_be(bytes, 4, frame.version);
        write_u16_be(bytes, 6, static_cast<std::uint16_t>(frame.type));
        write_u32_be(bytes, 8, frame.request_id);
        write_u32_be(
            bytes,
            12,
            static_cast<std::uint32_t>(frame.payload.size()));

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

        const std::uint16_t version = read_u16_be(bytes, 4);

        if (version != VERSION)
        {
            throw std::invalid_argument("Unsupported protocol version");
        }

        const std::uint32_t payload_size = read_u32_be(bytes, 12);

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
        frame.type = static_cast<MessageType>(read_u16_be(bytes, 6));
        frame.request_id = read_u32_be(bytes, 8);

        frame.payload.assign(
            bytes.begin() + static_cast<std::ptrdiff_t>(HEADER_SIZE),
            bytes.end());

        return frame;
    }

} // namespace hmq::protocol