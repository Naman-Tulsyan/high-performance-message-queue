#pragma once

#include "hmq/message.hpp"

#include <filesystem>
#include <vector>
#include <string>
#include <cstdint>

class MessageLog
{
private:
    std::filesystem::path path_;

public:
    explicit MessageLog(const std::filesystem::path &path);
    void append(const std::string &content);

    std::vector<Message> read_all() const;
};
