#include "hmq/consumer.hpp"

#include "hmq/protocol.hpp"
#include "hmq/tcp_client.hpp"

#include <stdexcept>
#include <utility>
#include <vector>

namespace hmq
{
    struct Consumer::Impl
    {
        TcpClient client;
        std::uint32_t next_request_id = 1;

        Impl(const std::string &host, std::uint16_t port) : client(host, port)
        {
        }
    };

    Consumer::Consumer(const std::string &host, std::uint16_t port) : impl_(std::make_unique<Impl>(host, port))
    {
    }

    Consumer::~Consumer() = default;

    Consumer::Consumer(Consumer &&) noexcept = default;
    Consumer &Consumer::operator=(Consumer &&) noexcept = default;

    std::optional<std::string> Consumer::consume(std::uint64_t offset)
    {
        protocol::Frame request;
        request.type = protocol::MessageType::CONSUME;
        request.request_id = impl_->next_request_id++;
        request.payload = protocol::encode_offset(offset);

        const auto response = impl_->client.request(request);

        if (response.type == protocol::MessageType::ERROR)
        {
            const auto code = protocol::decode_error_code(response.payload);
            if (code == protocol::ErrorCode::OFFSET_NOT_FOUND)
            {
                return std::nullopt;
            }

            throw std::runtime_error("Consume rejected, error code: " + std::to_string(static_cast<std::uint16_t>(code)));
        }

        if (response.type != protocol::MessageType::CONSUME_RESPONSE)
        {
            throw std::runtime_error("Unexpected consume response");
        }

        if (response.payload.size() < 8)
        {
            throw std::runtime_error("Malformed consume response");
        }

        const std::vector<std::uint8_t> offset_bytes(response.payload.begin(), response.payload.begin() + 8);
        if (protocol::decode_offset(offset_bytes) != offset)
        {
            throw std::runtime_error("Consume response offset mismatch");
        }

        return std::string(response.payload.begin() + 8, response.payload.end());
    }
}
