
#include "hmq/protocol.hpp"
#include "hmq/tcp_client.hpp"
#include "support/mock_server.hpp"

#include <array>
#include <cstdint>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

namespace
{
    using hmq::test::Bytes;
    using hmq::test::MockServer;
    
    Bytes make_ping_response(std::uint32_t request_id)
    {
        hmq::protocol::Frame frame;
        frame.type = hmq::protocol::MessageType::PING_RESPONSE;
        frame.request_id = request_id;

        return hmq::protocol::encode_frame(frame);
    }

    hmq::protocol::Frame make_request()
    {
        hmq::protocol::Frame request;
        request.type = hmq::protocol::MessageType::PING;
        request.request_id = 42;
        return request;
    }

    void expect_request_failure(const std::string &test_name, const Bytes &server_response)
    {
        MockServer server(server_response);
        hmq::TcpClient client("127.0.0.1", server.port());

        bool threw = false;

        try
        {
            (void)client.request(make_request());
        }
        catch (const std::exception &)
        {
            threw = true;
        }

        if (!threw)
        {
            throw std::runtime_error(test_name + ": expected client.request() to throw");
        }

        std::cout << "PASS: " << test_name << '\n';
    }

    void test_mismatched_request_id()
    {
        // A valid response frame, but for a different request.
        expect_request_failure(
            "mismatched response request ID",
            make_ping_response(999));
    }

    void test_incomplete_response()
    {
        // Send only part of a valid header, then disconnect.
        auto response = make_ping_response(42);
        response.resize(8);

        expect_request_failure(
            "incomplete response",
            response);
    }

    void test_malformed_response()
    {
        // Invalid magic bytes; the rest of the header is present.
        Bytes response(16, 0);
        response[0] = 'B';
        response[1] = 'A';
        response[2] = 'D';
        response[3] = '!';

        expect_request_failure(
            "malformed response header",
            response);
    }

} // namespace

int main()
{
    try
    {
        test_mismatched_request_id();
        test_incomplete_response();
        test_malformed_response();

        std::cout << "All TCP client tests passed.\n";
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
