#include <iostream>

#include "atom/memory/object.hpp"

// 定义一个简单的对象类
class MyObject {
public:
    MyObject(int id) : id(id) {
        std::cout << "MyObject " << id << " created." << std::endl;
    }

    ~MyObject() {
        std::cout << "MyObject " << id << " destroyed." << std::endl;
    }

    void doSomething() {
        std::cout << "MyObject " << id << " is doing something." << std::endl;
    }

    void reset() { std::cout << "MyObject " << id << " reset." << std::endl; }

private:
    int id;
};

int main() {
    // 创建一个 ObjectPool 对象
    ObjectPool<MyObject> pool(5);  // 假设池的大小为 5

    // 从对象池中获取对象并使用
    auto obj1 = pool.acquire();
    obj1->doSomething();

    auto obj2 = pool.acquire();
    obj2->doSomething();

    // 将对象归还到对象池中
    pool.release(std::move(obj1));
    pool.release(std::move(obj2));

    // 再次从对象池中获取对象并使用
    auto obj3 = pool.acquire();
    obj3->doSomething();

    // 将对象归还到对象池中
    pool.release(std::move(obj3));

    return 0;
}