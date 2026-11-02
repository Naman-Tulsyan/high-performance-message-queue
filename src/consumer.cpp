#include "hmq/consumer.hpp"
#include "hmq/protocol.hpp"

#include <stdexcept>

namespace hmq
{
    Consumer::Consumer(const std::string &host, std::uint16_t port) : client_(host, port)
    {
    }

    std::optional<std::string> Consumer::consume(std::uint64_t offset)
    {
        protocol::Frame request;
        request.type = protocol::MessageType::CONSUME;
        request.request_id = next_request_id_++;
        request.payload = protocol::encode_offset(offset);

        const auto response = client_.request(request);

        if (response.type == protocol::MessageType::ERROR)
        {
            const auto code = protocol::decode_error_code(response.payload);

            if (code == protocol::ErrorCode::OFFSET_NOT_FOUND)
            {
                return std::nullopt;
            }

            throw std::runtime_error("Broker rejected consume request");
        }

        if (response.type != protocol::MessageType::CONSUME_RESPONSE)
        {
            throw std::runtime_error("Unexpected consume response");
        }

        if (response.payload.size() < 8)
        {
            throw std::runtime_error("Malformed consume response");
        }

        return std::string(response.payload.begin() + 8, response.payload.end());
    }
}
