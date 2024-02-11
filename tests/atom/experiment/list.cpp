#include "atom/experiment/list.hpp"

int main()
{
    Deque<int> deque;

    deque.push_back(1);
    deque.push_front(2);

    if (auto value = deque.pop_front(); value)
    {
        std::cout << "Popped from front: " << *value << std::endl;
    }

    if (auto value = deque.pop_back(); value)
    {
        std::cout << "Popped from back: " << *value << std::endl;
    }

    deque.push_back(3);
    deque.push_front(4);

    std::cout << "Size of deque: " << deque.get_size() << std::endl;

    if (auto index = deque.find(3); index)
    {
        std::cout << "Found value 3 at index: " << *index << std::endl;
    }

    deque.insert(1, 5);
    deque.remove_at(2);

    deque.reverse_traversal();

    Deque<int> otherDeque;
    otherDeque.push_back(6);
    otherDeque.push_front(7);

    deque.concatenate(otherDeque);

    std::cout << "Size of concatenated deque: " << deque.get_size() << std::endl;

    for (DequeIterator<int> it = deque.begin(); it != deque.end(); ++it)
    {
        int value = *it;
        std::cout << value << std::endl;
        // 处理元素
    }

    return 0;
}