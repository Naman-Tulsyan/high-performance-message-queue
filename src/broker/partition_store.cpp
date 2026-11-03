
#include "hmq/partition_store.hpp"

#include <filesystem>
#include <stdexcept>
#include <utility>
#include <fstream>
#include <memory>

namespace hmq
{
    PartitionStore::PartitionStore(std::filesystem::path data_directory) : paths_(std::move(data_directory))
    {
    }

    PartitionStore::Partition &PartitionStore::get_or_create(const TopicPartition &tp)
    {
        tp.validate();

        const auto key = std::make_pair(tp.topic, tp.partition);

        auto it = partitions_.find(key);
        if (it != partitions_.end())
        {
            return it->second;
        }

        const auto path = paths_.log_path(tp);

        std::filesystem::create_directories(path.parent_path());

        // Ensure the log file exists before recovery.
        {
            std::ofstream file(path, std::ios::binary | std::ios::app);
            if (!file)
            {
                throw std::runtime_error("Failed to create partition log");
            }
        }

        PartitionStore::Partition partition;
        partition.log = std::make_unique<MessageLog>(path);

        partition.log->recover();
        partition.messages = partition.log->read_all();

        auto [inserted, success] = partitions_.emplace(key, std::move(partition));
        return inserted->second;
    }

    std::uint64_t PartitionStore::append(const TopicPartition &tp, const std::string &content)
    {
        Partition &partition = get_or_create(tp);

        const std::uint64_t offset = partition.messages.size();

        partition.log->append(content);

        partition.messages.push_back(
            Message{
                static_cast<std::size_t>(offset),
                content});

        return offset;
    }

    std::optional<std::string> PartitionStore::read(const TopicPartition &tp, std::uint64_t offset)
    {
        Partition &partition = get_or_create(tp);

        if (offset >= partition.messages.size())
        {
            return std::nullopt;
        }

        return partition.messages[static_cast<std::size_t>(offset)].content;
    }
}
