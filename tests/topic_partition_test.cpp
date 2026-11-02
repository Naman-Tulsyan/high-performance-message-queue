
#include "hmq/topic_partition.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>

namespace
{
    template <typename Function>
    void expect_invalid(Function function)
    {
        bool threw = false;

        try
        {
            function();
        }
        catch (const std::invalid_argument &)
        {
            threw = true;
        }

        if (!threw)
        {
            throw std::runtime_error("Expected invalid_argument");
        }
    }

    void test_valid_topic_partition()
    {
        const hmq::TopicPartition tp("orders", 0);

        assert(tp.topic == "orders");
        assert(tp.partition == 0);

        const hmq::TopicPartition another("orders", 0);
        const hmq::TopicPartition different("orders", 1);

        assert(tp == another);
        assert(!(tp == different));

        const hmq::TopicPartition max_partition("payments_2026", UINT32_MAX);

        assert(max_partition.partition == UINT32_MAX);
    }

    void test_invalid_topic_names()
    {
        expect_invalid([]
                       { hmq::TopicPartition tp("", 0); });

        expect_invalid([]
                       { hmq::TopicPartition tp("orders/payments", 0); });

        expect_invalid([]
                       { hmq::TopicPartition tp("../logs", 0); });

        expect_invalid([]
                       { hmq::TopicPartition tp("orders.v2", 0); });

        expect_invalid([]
                       { hmq::TopicPartition tp(std::string(256, 'a'), 0); });
    }

    void test_boundary_topic_names()
    {
        const hmq::TopicPartition tp(std::string(255, 'a'), 0);
        assert(tp.topic.size() == 255);

        const hmq::TopicPartition tp2("Orders-2026_1", 42);
        assert(tp2.partition == 42);
    }
}

int main()
{
    try
    {
        test_valid_topic_partition();
        test_invalid_topic_names();
        test_boundary_topic_names();

        std::cout << "All topic partition tests passed\n";
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
