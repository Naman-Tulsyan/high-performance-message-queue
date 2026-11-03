
#pragma once

#include <cstdint>

namespace hmq::byte_order
{
    inline void write_u16_be(std::uint8_t *bytes, std::uint16_t value)
    {
        bytes[0] = static_cast<std::uint8_t>(value >> 8);
        bytes[1] = static_cast<std::uint8_t>(value);
    }

    inline void write_u32_be(std::uint8_t *bytes, std::uint32_t value)
    {
        bytes[0] = static_cast<std::uint8_t>(value >> 24);
        bytes[1] = static_cast<std::uint8_t>(value >> 16);
        bytes[2] = static_cast<std::uint8_t>(value >> 8);
        bytes[3] = static_cast<std::uint8_t>(value);
    }

    inline void write_u64_be(std::uint8_t *bytes, std::uint64_t value)
    {
        for (int i = 7; i >= 0; --i)
        {
            bytes[i] = static_cast<std::uint8_t>(value);
            value >>= 8;
        }
    }

    inline std::uint16_t read_u16_be(const std::uint8_t *bytes)
    {
        return (static_cast<std::uint16_t>(bytes[0]) << 8) |
               static_cast<std::uint16_t>(bytes[1]);
    }

    inline std::uint32_t read_u32_be(const std::uint8_t *bytes)
    {
        return (static_cast<std::uint32_t>(bytes[0]) << 24) |
               (static_cast<std::uint32_t>(bytes[1]) << 16) |
               (static_cast<std::uint32_t>(bytes[2]) << 8) |
               static_cast<std::uint32_t>(bytes[3]);
    }

    inline std::uint64_t read_u64_be(const std::uint8_t *bytes)
    {
        std::uint64_t value = 0;

        for (int i = 0; i < 8; ++i)
        {
            value = (value << 8) | bytes[i];
        }

        return value;
    }
}
