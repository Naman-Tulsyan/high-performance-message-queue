
#include "hmq/broker.hpp"

#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <string>

namespace hmq
{
    Broker::Broker(const std::filesystem::path &log_path) : log_(log_path)
    {
        // MessageLog::recover() currently requires an existing file.
        if (!std::filesystem::exists(log_path))
        {
            std::ofstream file(log_path, std::ios::binary);
            if (!file)
            {
                throw std::runtime_error("Failed to create broker log");
            }
        }

        log_.recover();
        messages_ = log_.read_all();
    }

    protocol::Frame Broker::handle(const protocol::Frame &request)
    {
        using protocol::MessageType;

        protocol::Frame response;
        response.request_id = request.request_id;

        const auto error = [&](protocol::ErrorCode code, const std::string &message)
        {
            response.type = MessageType::ERROR;
            response.payload = protocol::encode_error(code, message);
            return response;
        };

        switch (request.type)
        {
        case MessageType::PING:
            response.type = MessageType::PING_RESPONSE;
            return response;

        case MessageType::PUBLISH:
        {
            const std::uint64_t offset = messages_.size();

            if (request.payload.size() > protocol::MAX_PAYLOAD_SIZE - 8)
            {
                return error(protocol::ErrorCode::MESSAGE_TOO_LARGE, "Message exceeds publish limit");
            }

            const std::string content(request.payload.begin(), request.payload.end());
            log_.append(content);

            messages_.push_back({static_cast<std::size_t>(offset), content});

            response.type = MessageType::PUBLISH_ACK;
            response.payload = protocol::encode_offset(offset);
            return response;
        }

        case MessageType::CONSUME:
        {
            std::uint64_t offset = 0;

            try
            {
                offset = protocol::decode_offset(request.payload);
            }
            catch (const std::invalid_argument &)
            {
                return error(protocol::ErrorCode::INVALID_PAYLOAD, "Invalid consume payload");
            }

            if (offset >= messages_.size())
            {
                return error(protocol::ErrorCode::OFFSET_NOT_FOUND, "Offset not found");
            }

            const auto &message = messages_[static_cast<std::size_t>(offset)];

            // 8 bytes for offset + message bytes.
            if (message.content.size() > protocol::MAX_PAYLOAD_SIZE - 8)
            {
                return error(protocol::ErrorCode::MESSAGE_TOO_LARGE, "Message exceeds consume response limit");
            }

            response.type = MessageType::CONSUME_RESPONSE;
            response.payload = protocol::encode_offset(offset);
            response.payload.insert(
                response.payload.end(),
                message.content.begin(),
                message.content.end());

            return response;
        }

        default:
            return error(protocol::ErrorCode::UNKNOWN_REQUEST, "Unsupported request");
        }
    }
}
