#pragma once

#include "message.hpp"

#include <queue>
#include <cstddef>
#include <optional>
#include <string>

class MessageQueue
{
private:
    std::queue<Message> messages_;
    std::size_t next_offset_ = 0;

public:
    std::size_t push(string content);
    std::optional<Message> try_pop();

    std::size_t size() const;
    bool empty() const;
};
