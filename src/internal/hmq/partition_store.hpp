#pragma once

#include "hmq/message_log.hpp"
#include "hmq/partition_paths.hpp"
#include "hmq/topic_partition.hpp"

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace hmq
{
    class PartitionStore
    {
    private:
        struct Partition
        {
            // unique_ptr is used to manage the lifetime of MessageLog instances, ensuring proper cleanup when a Partition is destroyed.
            std::unique_ptr<MessageLog> log;
            std::vector<Message> messages;
        };

        // & with get_or_create indicates that this function returns a reference to a Partition object, allowing direct access and modification of the Partition within the partitions_ map.
        Partition &get_or_create(const TopicPartition &tp);

        PartitionPaths paths_;

        // The map key is (topic, partition)
        std::map<std::pair<std::string, std::uint32_t>, Partition> partitions_;

    public:
        explicit PartitionStore(std::filesystem::path data_directory);

        std::uint64_t append(const TopicPartition &tp, const std::string &content);

        std::optional<std::string> read(const TopicPartition &tp, std::uint64_t offset);
    };
}
