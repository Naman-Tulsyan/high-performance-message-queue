
#include "hmq/protocol.hpp"
#include "hmq/tcp_client.hpp"

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
    using Bytes = std::vector<std::uint8_t>;

    void send_all(int fd, const Bytes &bytes)
    {
        std::size_t sent = 0;

        while (sent < bytes.size())
        {
            const auto result = send(fd, bytes.data() + sent, bytes.size() - sent, 0);

            if (result < 0)
            {
                throw std::runtime_error("Mock server send failed");
            }
            if (result == 0)
            {
                throw std::runtime_error("Mock server sent zero bytes");
            }

            sent += static_cast<std::size_t>(result);
        }
    }

    class MockServer
    {
    public:
        explicit MockServer(Bytes response)
        {
            listener_ = socket(AF_INET, SOCK_STREAM, 0);

            if (listener_ < 0)
            {
                throw std::runtime_error("socket failed");
            }

            sockaddr_in address{};
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            address.sin_port = 0; // Let the OS choose an available port.

            if (bind(listener_, reinterpret_cast<sockaddr *>(&address), sizeof(address)) < 0)
            {
                close(listener_);
                listener_ = -1;
                throw std::runtime_error("bind failed");
            }

            if (listen(listener_, 1) < 0)
            {
                close(listener_);
                listener_ = -1;
                throw std::runtime_error("listen failed");
            }

            socklen_t length = sizeof(address);

            // since OS chooses the port, we need to retrieve it using getsockname
            if (getsockname(listener_, reinterpret_cast<sockaddr *>(&address), &length) < 0)
            {
                close(listener_);
                listener_ = -1;
                throw std::runtime_error("getsockname failed");
            }

            port_ = ntohs(address.sin_port);

            worker_ = std::thread([this, response = std::move(response)]
                                  {
                const int client_fd = accept(listener_, nullptr, nullptr);

                if (client_fd < 0) {
                    return;
                }

                try {
                    send_all(client_fd, response);
                } catch (...) {
                    // The test observes the client's result.
                }

                shutdown(client_fd, SHUT_RDWR);
                close(client_fd); });
        }

        ~MockServer()
        {
            if (worker_.joinable()) // checks if worker_ thread is joinable (i.e., it has not been joined or detached yet). If it is joinable, we call join() to wait for the thread to finish executing before proceeding. This ensures that the thread completes its work and any resources associated with it are properly cleaned up before the MockServer object is destroyed.
            {
                worker_.join();
            }

            if (listener_ >= 0)
            {
                close(listener_);
            }
        }

        MockServer(const MockServer &) = delete;
        MockServer &operator=(const MockServer &) = delete;

        std::uint16_t port() const
        {
            return port_;
        }

    private:
        int listener_ = -1;
        std::uint16_t port_ = 0;
        std::thread worker_;
    };

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
