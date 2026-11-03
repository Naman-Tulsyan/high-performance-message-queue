#include "hmq/producer.hpp"

#include "hmq/protocol.hpp"
#include "hmq/tcp_client.hpp"

#include <stdexcept>
#include <utility>

namespace hmq
{
    struct Producer::Impl
    {
        TcpClient client;
        std::uint32_t next_request_id = 1;

        Impl(const std::string &host, std::uint16_t port) : client(host, port)
        {
        }
    };

    Producer::Producer(const std::string &host, std::uint16_t port) : impl_(std::make_unique<Impl>(host, port))
    {
    }

    Producer::~Producer() = default;

    Producer::Producer(Producer &&) noexcept = default;
    Producer &Producer::operator=(Producer &&) noexcept = default;

    std::uint64_t Producer::publish(const std::string &content)
    {
        protocol::Frame request;
        request.type = protocol::MessageType::PUBLISH;
        request.request_id = impl_->next_request_id++;
        request.payload.assign(content.begin(), content.end());

        const auto response = impl_->client.request(request);

        if (response.type == protocol::MessageType::ERROR)
        {
            const auto code = protocol::decode_error_code(response.payload);
            throw std::runtime_error("Publish rejected, error code: " + std::to_string(static_cast<std::uint16_t>(code)));
        }

        if (response.type != protocol::MessageType::PUBLISH_ACK)
        {
            throw std::runtime_error("Unexpected publish response");
        }

        return protocol::decode_offset(response.payload);
    }
}
