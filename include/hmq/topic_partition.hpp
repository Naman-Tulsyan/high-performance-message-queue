#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>

namespace hmq
{
    struct TopicPartition
    {
        std::string topic;
        std::uint32_t partition;

        TopicPartition(std::string topic_name, std::uint32_t partition_id) : topic(std::move(topic_name)), partition(partition_id)
        {
            if (topic.empty() || topic.size() > 255)
            {
                throw std::invalid_argument("Topic name must contain 1-255 characters");
            }

            for (const char c : topic)
            {
                const bool valid =
                    (c >= 'a' && c <= 'z') ||
                    (c >= 'A' && c <= 'Z') ||
                    (c >= '0' && c <= '9') ||
                    c == '_' || c == '-';

                if (!valid)
                {
                    throw std::invalid_argument("Invalid character in topic name");
                }
            }
        }

        bool operator==(const TopicPartition &) const = default;
    };
}
