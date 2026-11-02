#pragma once

#include "hmq/topic_partition.hpp"

#include <filesystem>
#include <stdexcept>
#include <string>

namespace hmq
{
    class PartitionPaths
    {
    private:
        std::filesystem::path data_directory_;

    public:
        explicit PartitionPaths(std::filesystem::path data_directory) : data_directory_(std::move(data_directory))
        {
            if (data_directory_.empty())
            {
                throw std::invalid_argument("Data directory cannot be empty");
            }
        }

        std::filesystem::path log_path(const TopicPartition &tp) const
        {
            // Revalidate because TopicPartition fields are mutable.
            tp.validate();

            return data_directory_ / tp.topic / ("partition-" + std::to_string(tp.partition) + ".log");
        }
    };
}
