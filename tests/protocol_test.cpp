#include "hmq/protocol.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>

using hmq::protocol::Frame;
using hmq::protocol::MessageType;

int main()
{
    // Round-trip a normal request.
    Frame original;
    original.type = MessageType::PING;
    original.request_id = 42;
    original.payload = {0x01, 0x02, 0x03};

    const auto encoded = hmq::protocol::encode_frame(original);

    assert(encoded.size() == hmq::protocol::HEADER_SIZE + 3);
    assert(encoded[0] == 'H');
    assert(encoded[1] == 'M');
    assert(encoded[2] == 'Q');
    assert(encoded[3] == '1');

    // Version 1 and PING type must be big-endian.
    assert(encoded[4] == 0x00 && encoded[5] == 0x01);
    assert(encoded[6] == 0x00 && encoded[7] == 0x01);

    const auto decoded = hmq::protocol::decode_frame(encoded);

    assert(decoded.version == original.version);
    assert(decoded.type == original.type);
    assert(decoded.request_id == original.request_id);
    assert(decoded.payload == original.payload);

    // Zero-length payload is valid.
    Frame empty;
    empty.type = MessageType::PING;
    const auto empty_bytes = hmq::protocol::encode_frame(empty);
    const auto empty_decoded = hmq::protocol::decode_frame(empty_bytes);

    assert(empty_decoded.payload.empty());

    // Incomplete header must fail.
    bool threw = false;

    try
    {
        hmq::protocol::decode_frame({0x48, 0x4D});
    }
    catch (const std::runtime_error &)
    {
        threw = true;
    }

    assert(threw);

    // Incorrect magic must fail.
    threw = false;

    auto bad_magic = encoded;
    bad_magic[0] = 0x00;

    try
    {
        hmq::protocol::decode_frame(bad_magic);
    }
    catch (const std::invalid_argument &)
    {
        threw = true;
    }

    assert(threw);

    // A declared payload size that doesn't match the bytes must fail.
    threw = false;

    auto bad_length = encoded;
    bad_length[15] = 0x04; // Declares 4 bytes; only 3 are present.

    try
    {
        hmq::protocol::decode_frame(bad_length);
    }
    catch (const std::runtime_error &)
    {
        threw = true;
    }

    assert(threw);

    // Unsupported versions must fail.
    threw = false;

    auto bad_version = encoded;
    bad_version[5] = 0x02;

    try
    {
        hmq::protocol::decode_frame(bad_version);
    }
    catch (const std::invalid_argument &)
    {
        threw = true;
    }

    assert(threw);

    std::cout << "All protocol tests passed\n";
    return 0;
}