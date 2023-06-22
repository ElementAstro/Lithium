#include <queue>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <atomic>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>

class Message
{
public:
    std::string messageId;
    // 其它消息字段

    explicit Message(std::string id) : messageId(std::move(id)) {}
};

class MessageQueue
{
private:
    std::queue<Message> queue_;
    std::unordered_map<std::string, bool> messageStatus_;
    std::mutex mutex_;
    std::condition_variable cv_;
    size_t maxCapacity_;
    std::atomic<bool> isShutdown_;

public:
    explicit MessageQueue(size_t maxCapacity = 1000)
        : maxCapacity_(maxCapacity),
          isShutdown_(false) {}

    void sendMessage(const Message &message)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]()
                 { return !isShutdown_ && queue_.size() < maxCapacity_; });

        if (!isShutdown_)
        {
            queue_.push(message);
            messageStatus_[message.messageId] = false; // 设置为未确认状态
            cv_.notify_one();
        }
    }

    bool receiveMessage(std::vector<Message> &messages, size_t batchSize = 10)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]()
                 { return isShutdown_ || !queue_.empty(); });

        const size_t count = std::min(batchSize, queue_.size());
        if (count > 0)
        {
            messages.reserve(count);
            for (size_t i = 0; i < count; ++i)
            {
                messages.push_back(std::move(queue_.front()));
                queue_.pop();
            }
            cv_.notify_one();
            return true;
        }

        return false;
    }

    void confirmMessage(const std::string &messageId)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        messageStatus_[messageId] = true;
    }

    void shutdown()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            isShutdown_ = true;
        }
        cv_.notify_all();
    }

    bool isEmpty() const
    {
        return queue_.empty();
    }

    size_t size() const
    {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex &>(mutex_));
        return queue_.size();
    }

    void monitorQueue(std::chrono::seconds interval) const
    {
        while (true)
        {
            std::this_thread::sleep_for(interval);
            std::lock_guard<std::mutex> lock(const_cast<std::mutex &>(mutex_));
            // 监控和报警逻辑
            std::cout << "Queue size: " << queue_.size() << std::endl;
        }
    }

    void processMessages()
    {
        while (true)
        {
            std::vector<Message> messages;
            if (receiveMessage(messages))
            {
                for (auto &message : messages)
                {
                    // 处理消息
                    if (processMessage(message))
                    {
                        confirmMessage(message.messageId);
                    }
                    else
                    {
                        retryMessage(message);
                    }
                }
            }
            else
            {
                break;
            }
        }
    }

    bool processMessage(Message &message)
    {
        // 处理消息逻辑
        // 返回处理结果，并根据实际情况进行确认或重试
        // 假设处理成功的概率为70%
        bool success = (std::rand() % 100) < 70;
        if (success)
        {
            // 处理成功，进行确认
            return true;
        }
        return false;
    }

    void retryMessage(const Message &message)
    {
        // 错误处理与重试逻辑
        // 根据具体情况进行重试或放弃处理
        // 假设最大重试次数为3次
        int maxRetries = 3;
        int retries = 0;
        bool success = false;
        while (retries < maxRetries && !success)
        {
            // 重试逻辑
            // 假设等待一段时间后重试
            std::this_thread::sleep_for(std::chrono::seconds(1));
            success = processMessage(const_cast<Message &>(message));
            retries++;
        }
        if (!success)
        {
            std::cout << "Message processing failed after " << maxRetries << " retries" << std::endl;
        }
    }
};

int main()
{
    MessageQueue messageQueue;

    // 创建生产者线程
    std::thread producerThread([&]()
                               {
        for (int i = 0; i < 10; ++i)
        {
            Message message("Message" + std::to_string(i));
            // 设置消息字段
            messageQueue.sendMessage(message);

            // 可选：添加适当的等待时间，限制生产速率
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        } });

    // 创建消费者线程
    std::thread consumerThread([&]()
                               { messageQueue.processMessages(); });

    // 创建监控线程
    std::thread monitorThread([&]()
                              { messageQueue.monitorQueue(std::chrono::seconds(10)); });

    // 主线程运行一段时间后关闭队列
    std::this_thread::sleep_for(std::chrono::seconds(30));
    messageQueue.shutdown();

    // 等待生产者、消费者和监控线程完成
    producerThread.join();
    consumerThread.join();
    monitorThread.join();

    return 0;
}
