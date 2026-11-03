#include "hmq/producer.hpp"
#include "hmq/protocol.hpp"

#include <stdexcept>

namespace hmq
{
    Producer::Producer(const std::string &host, std::uint16_t port) : client_(host, port)
    {
    }

    std::uint64_t Producer::publish(const std::string &content)
    {
        protocol::Frame request;
        request.type = protocol::MessageType::PUBLISH;
        request.request_id = next_request_id_++;

        request.payload.assign(content.begin(), content.end());

        const auto response = client_.request(request);

        if (response.type == protocol::MessageType::ERROR)
        {
            throw std::runtime_error("Broker rejected publish request");
        }

        if (response.type != protocol::MessageType::PUBLISH_ACK)
        {
            throw std::runtime_error("Unexpected publish response");
        }

        return protocol::decode_offset(response.payload);
    }
}
