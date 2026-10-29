
#include "hmq/protocol.hpp"
#include "hmq/tcp_client.hpp"

#include <chrono>  // for std::chrono::milliseconds
#include <csignal> // for kill
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace
{
    constexpr std::uint16_t PORT = 19090;

    class TestServer
    {
    public:
        explicit TestServer(const char *executable)
        {
            pid_ = fork();

            if (pid_ < 0)
            {
                throw std::runtime_error("fork failed");
            }

            if (pid_ == 0)
            {
                const std::string port = std::to_string(PORT);
                execl(executable, executable, port.c_str(), static_cast<char *>(nullptr));
                _exit(127);
            }

            // Wait until the server is accepting connections.
            for (int i = 0; i < 100; ++i)
            {
                int status = 0;
                const pid_t result = waitpid(pid_, &status, WNOHANG); // WNOHANG is a flag that can be passed to waitpid to indicate that the function should return immediately if the child process has not yet changed state (i.e., it is still running). This allows the parent process to continue executing without blocking while waiting for the child process to finish.

                if (result == pid_)
                {
                    pid_ = -1;
                    throw std::runtime_error("Server exited early. Check its port configuration.");
                }

                try
                {
                    hmq::TcpClient probe("127.0.0.1", PORT);
                    return;
                }
                catch (const std::exception &)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }
            }

            stop();
            throw std::runtime_error("Server startup timed out");
        }

        ~TestServer()
        {
            stop();
        }

        TestServer(const TestServer &) = delete;
        TestServer &operator=(const TestServer &) = delete;

    private:
        pid_t pid_ = -1;

        void stop()
        {
            if (pid_ <= 0)
            {
                return;
            }

            kill(pid_, SIGTERM);

            int status = 0;
            while (waitpid(pid_, &status, 0) < 0)
            {
                // Retry if interrupted by a signal.
            }

            pid_ = -1;
        }
    };

    void test_ping(hmq::TcpClient &client)
    {
        hmq::protocol::Frame request;
        request.type = hmq::protocol::MessageType::PING;
        request.request_id = 42;

        const auto response = client.request(request);

        if (response.type != hmq::protocol::MessageType::PING_RESPONSE ||
            response.request_id != request.request_id ||
            !response.payload.empty())
        {
            throw std::runtime_error("Invalid PING response");
        }

        std::cout << "PASS: PING/PING_RESPONSE\n";
    }

    void test_multiple_requests(hmq::TcpClient &client)
    {
        for (std::uint32_t id : {101U, 102U, 103U})
        {
            hmq::protocol::Frame request;
            request.type = hmq::protocol::MessageType::PING;
            request.request_id = id;

            const auto response = client.request(request);

            if (response.type != hmq::protocol::MessageType::PING_RESPONSE || response.request_id != id)
            {
                throw std::runtime_error("Invalid response to repeated PING");
            }
        }

        std::cout << "PASS: multiple requests on one connection\n";
    }

    void test_unsupported_request(hmq::TcpClient &client)
    {
        hmq::protocol::Frame request;
        request.type = static_cast<hmq::protocol::MessageType>(0x1234);
        request.request_id = 77;

        const auto response = client.request(request);

        const std::string payload(response.payload.begin(), response.payload.end());

        if (response.type != hmq::protocol::MessageType::ERROR ||
            response.request_id != request.request_id ||
            payload != "Unsupported request")
        {
            throw std::runtime_error("Invalid unsupported-request response");
        }

        std::cout << "PASS: unsupported request returns ERROR\n";
    }

} // namespace

int main(int argc, char *argv[]) // argv = [program name, server executable path] . eg. ./tcp_server_test ./hmq_server
{
    if (argc != 2)
    {
        std::cerr << "Usage: tcp_server_test <server-executable>\n";
        return 1;
    }

    try
    {
        TestServer server(argv[1]);
        hmq::TcpClient client("127.0.0.1", PORT);

        test_ping(client);
        test_multiple_requests(client);
        test_unsupported_request(client);

        std::cout << "All TCP server integration tests passed.\n";
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
