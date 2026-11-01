
#include "hmq/byte_order.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>

int main()
{
    std::array<std::uint8_t, 8> bytes{};

    // Zero
    hmq::byte_order::write_u64_be(bytes.data(), 0);
    assert(hmq::byte_order::read_u64_be(bytes.data()) == 0);

    // A value that crosses a byte boundary
    constexpr std::uint64_t value = 258;
    hmq::byte_order::write_u64_be(bytes.data(), value);

    assert(bytes[6] == 0x01);
    assert(bytes[7] == 0x02);
    assert(hmq::byte_order::read_u64_be(bytes.data()) == value);

    // Largest possible unsigned 64-bit value
    constexpr auto maximum = std::numeric_limits<std::uint64_t>::max();
    hmq::byte_order::write_u64_be(bytes.data(), maximum);

    assert(hmq::byte_order::read_u64_be(bytes.data()) == maximum);

    std::cout << "All byte-order tests passed\n";
    return 0;
}
