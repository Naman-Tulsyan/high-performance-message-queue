
#include "hmq/broker.hpp"

#include <cassert>
#include <filesystem>
#include <iostream>
#include <string>

int main()
{
    namespace fs = std::filesystem;
    using namespace hmq::protocol;

    const auto path = fs::temp_directory_path() / "hmq_broker_test.bin";

    fs::remove(path);

    {
        hmq::Broker broker(path);

        Frame publish;
        publish.type = MessageType::PUBLISH;
        publish.request_id = 10;
        publish.payload = {'h', 'e', 'l', 'l', 'o'};

        const auto ack = broker.handle(publish);

        assert(ack.type == MessageType::PUBLISH_ACK);
        assert(ack.request_id == 10);
        assert(decode_offset(ack.payload) == 0);

        Frame consume;
        consume.type = MessageType::CONSUME;
        consume.request_id = 11;
        consume.payload = encode_offset(0);

        const auto result = broker.handle(consume);

        assert(result.type == MessageType::CONSUME_RESPONSE);
        assert(result.request_id == 11);

        assert(result.payload.size() == 13);

        const std::string content(
            result.payload.begin() + 8,
            result.payload.end());

        assert(content == "hello");

        // Non-destructive consume.
        const auto again = broker.handle(consume);
        assert(again.payload == result.payload);
    }

    // Verify persistence after reopening the broker.
    {
        hmq::Broker broker(path);

        Frame consume;
        consume.type = MessageType::CONSUME;
        consume.payload = encode_offset(0);

        const auto response = broker.handle(consume);

        assert(response.type == MessageType::CONSUME_RESPONSE);

        const std::string content(
            response.payload.begin() + 8,
            response.payload.end());

        assert(content == "hello");
    }

    // Missing offset
    {
        hmq::Broker broker(path);

        Frame request;
        request.type = MessageType::CONSUME;
        request.request_id = 201;
        request.payload = encode_offset(999);

        const auto response = broker.handle(request);

        assert(response.type == MessageType::ERROR);
        assert(response.request_id == 201);
        assert(decode_error_code(response.payload) == ErrorCode::OFFSET_NOT_FOUND);
    }

    // Invalid consume payload
    {
        hmq::Broker broker(path);

        Frame request;
        request.type = MessageType::CONSUME;
        request.request_id = 202;
        request.payload = {0x01, 0x02};

        const auto response = broker.handle(request);

        assert(response.type == MessageType::ERROR);
        assert(decode_error_code(response.payload) == ErrorCode::INVALID_PAYLOAD);
    }

    // Unknown request type
    {
        hmq::Broker broker(path);

        Frame request;
        request.type = static_cast<MessageType>(0x1234);
        request.request_id = 203;

        const auto response = broker.handle(request);

        assert(response.type == MessageType::ERROR);
        assert(decode_error_code(response.payload) == ErrorCode::UNKNOWN_REQUEST);
    }

    // Oversized publish must be rejected
    {
        hmq::Broker broker(path);

        Frame request;
        request.type = MessageType::PUBLISH;
        request.request_id = 204;
        request.payload.resize(MAX_PAYLOAD_SIZE - 7, 'x');

        const auto response = broker.handle(request);

        assert(response.type == MessageType::ERROR);
        assert(decode_error_code(response.payload) == ErrorCode::MESSAGE_TOO_LARGE);
    }

    fs::remove(path);

    std::cout << "All broker tests passed\n";
}
