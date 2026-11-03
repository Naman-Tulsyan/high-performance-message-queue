#include "hmq/message_queue.hpp"

#include <utility>
using namespace std;

size_t MessageQueue::push(string content)
{
    const size_t offset = next_offset_++;
    messages_.push(Message{offset, std::move(content)});
    return offset;
}

optional<Message> MessageQueue::try_pop()
{
    if (messages_.empty())
    {
        return nullopt;
    }

    Message msg = messages_.front();
    messages_.pop();
    return msg;
}

size_t MessageQueue::size() const
{
    return messages_.size();
}

bool MessageQueue::empty() const
{
    return messages_.empty();
}
