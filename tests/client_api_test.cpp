
#include "hmq/consumer.hpp"
#include "hmq/producer.hpp"
#include "hmq/protocol.hpp"
#include "support/mock_server.hpp"

#include <cstdint>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
    using namespace hmq::protocol;
    using hmq::test::MockServer;

    template <typename Operation>
    void expect_failure(const std::string &test_name, const Frame &response, Operation operation)
    {
        MockServer server(encode_frame(response));

        bool threw = false;

        try
        {
            operation(server.port());
        }
        catch (const std::exception &)
        {
            threw = true;
        }

        if (!threw)
        {
            throw std::runtime_error(test_name + ": expected exception");
        }

        std::cout << "PASS: " << test_name << '\n';
    }

    Frame make_response(MessageType type, std::vector<std::uint8_t> payload)
    {
        Frame response;
        response.type = type;
        response.request_id = 1;
        response.payload = std::move(payload);
        return response;
    }

    void test_malformed_publish_ack()
    {
        const auto response = make_response(MessageType::PUBLISH_ACK, {0x00, 0x00, 0x00, 0x01});

        expect_failure(
            "malformed publish ACK",
            response,
            [](std::uint16_t port)
            {
                hmq::Producer producer("127.0.0.1", port);
                (void)producer.publish("hello");
            });
    }

    void test_unexpected_publish_response()
    {
        const auto response = make_response(MessageType::PING_RESPONSE, {});

        expect_failure(
            "unexpected publish response",
            response,
            [](std::uint16_t port)
            {
                hmq::Producer producer("127.0.0.1", port);
                (void)producer.publish("hello");
            });
    }

    void test_malformed_consume_response()
    {
        const auto response = make_response(MessageType::CONSUME_RESPONSE, {0x00, 0x01});

        expect_failure(
            "malformed consume response",
            response,
            [](std::uint16_t port)
            {
                hmq::Consumer consumer("127.0.0.1", port);
                (void)consumer.consume(0);
            });
    }

    void test_wrong_consume_offset()
    {
        auto payload = encode_offset(999);

        const std::string content = "wrong message";

        payload.insert(
            payload.end(),
            content.begin(),
            content.end());

        const auto response = make_response(
            MessageType::CONSUME_RESPONSE,
            std::move(payload));

        expect_failure(
            "wrong consume offset",
            response,
            [](std::uint16_t port)
            {
                hmq::Consumer consumer("127.0.0.1", port);
                (void)consumer.consume(0);
            });
    }

    void test_malformed_error_producer()
    {
        const auto response = make_response(
            MessageType::ERROR,
            {0x00});

        expect_failure(
            "malformed ERROR for producer",
            response,
            [](std::uint16_t port)
            {
                hmq::Producer producer("127.0.0.1", port);
                (void)producer.publish("hello");
            });
    }

    void test_malformed_error_consumer()
    {
        const auto response = make_response(
            MessageType::ERROR,
            {0x00});

        expect_failure(
            "malformed ERROR for consumer",
            response,
            [](std::uint16_t port)
            {
                hmq::Consumer consumer("127.0.0.1", port);
                (void)consumer.consume(0);
            });
    }

    void test_unexpected_consume_response()
    {
        const auto response = make_response(
            MessageType::PING_RESPONSE, {});

        expect_failure(
            "unexpected consume response",
            response,
            [](std::uint16_t port)
            {
                hmq::Consumer consumer("127.0.0.1", port);
                (void)consumer.consume(0);
            });
    }
}

int main()
{
    try
    {
        test_malformed_publish_ack();
        test_unexpected_publish_response();
        test_malformed_consume_response();
        test_wrong_consume_offset();
        test_malformed_error_producer();
        test_malformed_error_consumer();
        test_unexpected_consume_response();

        std::cout << "All client API tests passed\n";
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
