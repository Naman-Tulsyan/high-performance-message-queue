#include "hmq/partition_paths.hpp"

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

namespace fs = std::filesystem;

namespace
{
    void expect(bool condition, const std::string &message)
    {
        if (!condition)
        {
            throw std::runtime_error(message);
        }
    }

    void test_log_paths()
    {
        const hmq::PartitionPaths paths("data");

        expect(
            paths.log_path({"orders", 0}) == fs::path("data/orders/partition-0.log"),
            "Incorrect path for orders partition 0");

        expect(
            paths.log_path({"orders", 1}) == fs::path("data/orders/partition-1.log"),
            "Incorrect path for orders partition 1");

        expect(
            paths.log_path({"payments", 0}) == fs::path("data/payments/partition-0.log"),
            "Incorrect path for payments partition 0");
    }

    void test_unique_partition_paths()
    {
        const hmq::PartitionPaths paths("data");

        const auto first = paths.log_path({"orders", 0});
        const auto second = paths.log_path({"orders", 1});
        const auto third = paths.log_path({"payments", 0});

        expect(first != second, "Different partitions share a path");
        expect(first != third, "Different topics share a path");
    }

    void test_empty_data_directory()
    {
        bool threw = false;

        try
        {
            const hmq::PartitionPaths paths("");
        }
        catch (const std::invalid_argument &)
        {
            threw = true;
        }

        expect(threw, "Empty data directory should be rejected");
    }

    void test_mutated_topic_is_rejected()
    {
        const hmq::PartitionPaths paths("data");

        hmq::TopicPartition tp("orders", 0);

        // Bypass constructor validation intentionally.
        tp.topic = "../outside";

        bool threw = false;

        try
        {
            (void)paths.log_path(tp);
        }
        catch (const std::invalid_argument &)
        {
            threw = true;
        }

        expect(threw, "Mutated invalid topic was accepted");
    }

    void test_max_partition_id()
    {
        const hmq::PartitionPaths paths("data");

        expect(
            paths.log_path({"orders", UINT32_MAX}) == fs::path("data/orders/partition-4294967295.log"),
            "Incorrect path for maximum partition ID");
    }
}

int main()
{
    try
    {
        test_log_paths();
        test_unique_partition_paths();
        test_empty_data_directory();
        test_mutated_topic_is_rejected();
        test_max_partition_id();

        std::cout << "All partition path tests passed\n";
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
