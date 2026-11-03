#include "hmq/message_queue.hpp"

#include <cassert>
#include <iostream>

int main()
{
    MessageQueue queue;

    // Empty queue
    assert(queue.empty());
    assert(queue.size() == 0);
    assert(!queue.try_pop().has_value());

    // Push assigns increasing offsets
    assert(queue.push("Order A") == 0);
    assert(queue.push("Order B") == 1);
    assert(queue.push("Order C") == 2);
    assert(queue.size() == 3);

    // FIFO order and message contents
    auto first = queue.try_pop();
    assert(first.has_value());
    assert(first->offset == 0);
    assert(first->content == "Order A");

    auto second = queue.try_pop();
    assert(second.has_value());
    assert(second->offset == 1);
    assert(second->content == "Order B");

    auto third = queue.try_pop();
    assert(third.has_value());
    assert(third->offset == 2);
    assert(third->content == "Order C");

    assert(queue.empty());

    // Offsets must not reset after popping
    assert(queue.push("Order D") == 3);

    auto fourth = queue.try_pop();
    assert(fourth.has_value());
    assert(fourth->offset == 3);
    assert(fourth->content == "Order D");

    // Each queue has its own offset sequence
    MessageQueue another_queue;
    assert(another_queue.push("Independent order") == 0);

    std::cout << "All MessageQueue tests passed\n";
    return 0;
}