
#include "hmq/partition_store.hpp"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <optional>
#include <cstdint>

namespace fs = std::filesystem;

namespace
{
    void expect(bool condition, const std::string& message)
    {
        if (!condition)
            throw std::runtime_error(message);
    }

    void test_partition_isolation_and_recovery()
    {
        const auto directory = fs::temp_directory_path() / ("hmq_partition_store_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));

        const hmq::TopicPartition orders0("orders", 0);
        const hmq::TopicPartition orders1("orders", 1);
        const hmq::TopicPartition payments0("payments", 0);

        try
        {
            {
                hmq::PartitionStore store(directory);

                expect(store.append(orders0, "A") == 0, "orders/0 first offset incorrect");

                expect(store.append(orders0, "B") == 1, "orders/0 second offset incorrect");

                expect(store.append(orders1, "C") == 0, "orders/1 first offset incorrect");

                expect(store.append(payments0, "D") == 0, "payments/0 first offset incorrect");

                expect(store.read(orders0, 0) == std::optional<std::string>("A"), "orders/0 read mismatch");

                expect(store.read(orders1, 0) == std::optional<std::string>("C"), "orders/1 read mismatch");

                expect(!store.read(orders1, 1).has_value(), "Unexpected message in orders/1");
            }

            // Simulate restarting the storage component.
            {
                hmq::PartitionStore store(directory);

                expect(store.read(orders0, 1) == std::optional<std::string>("B"), "orders/0 recovery failed");

                expect(store.read(payments0, 0) == std::optional<std::string>("D"), "payments/0 recovery failed");

                expect(store.append(orders0, "E") == 2, "Offset not preserved after restart");
            }

            fs::remove_all(directory);
        }
        catch (...)
        {
            fs::remove_all(directory);
            throw;
        }
    }
}

int main()
{
    try
    {
        test_partition_isolation_and_recovery();
        std::cout << "All partition store tests passed\n";
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
