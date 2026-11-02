
#include "hmq/producer.hpp"
#include "hmq/consumer.hpp"
#include "hmq/tcp_client.hpp"

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

#include <csignal>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace
{
    constexpr std::uint16_t PORT = 19091;

    class TestServer
    {
    public:
        TestServer(const char *executable, const std::filesystem::path &log_path)
        {
            pid_ = fork();

            if (pid_ < 0)
                throw std::runtime_error("fork failed");

            if (pid_ == 0)
            {
                const auto port = std::to_string(PORT);
                const auto path = log_path.string();

                execl(executable, executable, port.c_str(), path.c_str(), static_cast<char *>(nullptr));
                _exit(127);
            }

            for (int i = 0; i < 100; ++i)
            {
                int status = 0;

                if (waitpid(pid_, &status, WNOHANG) == pid_)
                {
                    pid_ = -1;
                    throw std::runtime_error("Server exited early");
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

        ~TestServer() { stop(); }

        TestServer(const TestServer &) = delete;
        TestServer &operator=(const TestServer &) = delete;

    private:
        pid_t pid_ = -1;

        void stop()
        {
            if (pid_ <= 0)
                return;

            kill(pid_, SIGTERM);

            int status = 0;
            while (waitpid(pid_, &status, 0) < 0)
            {
                // Retry on interrupted wait.
            }

            pid_ = -1;
        }
    };
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: producer_consumer_test <server>\n";
        return 1;
    }

    namespace fs = std::filesystem;

    const auto log_path = fs::temp_directory_path() / "hmq_producer_consumer_test.bin";

    try
    {
        fs::remove(log_path);

        {
            TestServer server(argv[1], log_path);

            std::uint64_t first = 0;
            std::uint64_t second = 0;
            std::uint64_t empty_offset = 0;

            // STEP 1: Publish messages.
            {
                hmq::Producer producer("127.0.0.1", PORT);

                first = producer.publish("hello");
                second = producer.publish("world");
                empty_offset = producer.publish("");

            } // Producer destructor closes its TCP connection.

            // STEP 2: Consume messages.
            {
                hmq::Consumer consumer("127.0.0.1", PORT);

                if (first != 0 || second != 1 || empty_offset != 2)
                    throw std::runtime_error("Incorrect publish offsets");

                if (consumer.consume(first) != std::optional<std::string>("hello"))
                    throw std::runtime_error("First message mismatch");

                if (consumer.consume(second) != std::optional<std::string>("world"))
                    throw std::runtime_error("Second message mismatch");

                if (consumer.consume(999).has_value())
                    throw std::runtime_error("Expected missing offset to return nullopt");

                if (consumer.consume(empty_offset) != std::optional<std::string>(""))
                    throw std::runtime_error("Empty message mismatch");

            } // Consumer destructor closes its TCP connection.

            std::cout << "PASS: publish and consume over TCP\n";
        }

        // Restart broker with the same log.
        {
            TestServer server(argv[1], log_path);

            hmq::Consumer consumer("127.0.0.1", PORT);

            if (consumer.consume(0) != std::optional<std::string>("hello"))
                throw std::runtime_error("Restart recovery failed");

            std::cout << "PASS: persistence after restart\n";
        }

        fs::remove(log_path);

        std::cout << "All producer/consumer tests passed\n";
        return 0;
    }
    catch (const std::exception &e)
    {
        fs::remove(log_path);
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
