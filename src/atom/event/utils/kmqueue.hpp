/*
 * kmqueue.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-10

Description: Memory Queue

*************************************************/

#pragma once

#include <memory>
#include <atomic>

namespace Atom::Event
{

    const size_t kPaddingSize = 128;
    ///
    // enqueue on a thread and dequene on another thread
    ///
    template <class E>
    class KMQueue
    {
    public:
        KMQueue()
        {
            head_ = new TLNode();
            tail_ = head_;
        }
        ~KMQueue()
        {
            TLNode *node = nullptr;
            while (head_)
            {
                node = head_;
                head_ = head_->next_.load(std::memory_order_relaxed);
                delete node;
            }
        }

        template <class... Args>
        void enqueue(Args &&...args)
        {
            TLNode *node = new TLNode(std::forward<Args>(args)...);
            tail_->next_.store(node, std::memory_order_release);
            tail_ = node;
            ++count_;
        }

        bool dequeue(E &element)
        {
            auto *node = head_->next_.load(std::memory_order_acquire);
            if (node == nullptr)
            {
                return false;
            }
            --count_;
            element = std::move(node->element_);
            delete head_;
            head_ = node;
            return true;
        }

        E &front()
        {
            auto *node = head_->next_.load(std::memory_order_acquire);
            if (node == nullptr)
            {
                static E E_empty{};
                return E_empty;
            }
            return node->element_;
        }

        void pop_front()
        {
            auto *node = head_->next_.load(std::memory_order_acquire);
            if (node != nullptr)
            {
                --count_;
                delete head_;
                head_ = node;
            }
        }

        bool empty()
        {
            return size() == 0;
        }

        size_t size()
        {
            return count_.load(std::memory_order_relaxed);
        }

    protected:
        class TLNode
        {
        public:
            template <class... Args>
            TLNode(Args &&...args) : element_{std::forward<Args>(args)...} {}

            E element_;
            std::atomic<TLNode *> next_{nullptr};
        };

        TLNode *head_{nullptr};
        char __pad0__[kPaddingSize - sizeof(TLNode *)];
        TLNode *tail_{nullptr};
        char __pad1__[kPaddingSize - sizeof(TLNode *)];
        std::atomic<size_t> count_{0};
    };

    // double linked list
    template <class E>
    class DLQueue final
    {
    public:
        class DLNode
        {
        public:
            using Ptr = std::shared_ptr<DLNode>;

            template <class... Args>
            DLNode(Args &&...args) : element_{std::forward<Args>(args)...} {}
            E &element() { return element_; }
            bool isLinked() const { return linked_; }

        private:
            friend class DLQueue;
            E element_;
            bool linked_{false};
            Ptr prev_;
            Ptr next_;
        };
        using NodePtr = typename DLNode::Ptr;

    public:
        ~DLQueue()
        {
            while (head_)
            {
                head_->linked_ = false;
                head_ = head_->next_;
            }
        }

        template <class... Args>
        NodePtr enqueue(Args &&...args)
        {
            auto node = std::make_shared<DLNode>(std::forward<Args>(args)...);
            return enqueue(node);
        }

        NodePtr enqueue(NodePtr &node)
        {
            if (empty())
            {
                head_ = node;
            }
            else
            {
                tail_->next_ = node;
                node->prev_ = tail_;
            }
            tail_ = node;
            node->linked_ = true;
            ++count_;
            return node;
        }

        bool dequeue(E &element)
        {
            if (empty())
            {
                return false;
            }
            element = std::move(head_->element_);
            pop_front();
            return true;
        }

        E &front()
        {
            if (empty())
            {
                static E E_empty{};
                return E_empty;
            }
            return head_->element_;
        }

        NodePtr &front_node()
        {
            return head_;
        }

        void pop_front()
        {
            if (!empty())
            {
                head_->linked_ = false;
                if (head_->next_)
                {
                    head_ = head_->next_;
                    head_->prev_->next_.reset();
                    head_->prev_.reset();
                }
                else
                {
                    head_.reset();
                    tail_.reset();
                }
                --count_;
            }
        }

        bool remove(const NodePtr &node)
        { // make sure the node is in this queue
            if (!node || (!node->prev_ && !node->next_ && node != head_))
            {
                return false;
            }
            if (node->next_)
            {
                node->next_->prev_ = node->prev_;
            }
            else if (tail_ == node)
            {
                tail_ = node->prev_;
            }
            if (node->prev_)
            {
                node->prev_->next_ = node->next_;
            }
            else if (head_ == node)
            {
                head_ = node->next_;
            }
            node->next_.reset();
            node->prev_.reset();
            node->linked_ = false;
            --count_;
            return true;
        }

        bool empty()
        {
            return !head_;
        }

        size_t size()
        {
            return count_.load(std::memory_order_relaxed);
        }

        void swap(DLQueue &other)
        {
            head_.swap(other.head_);
            tail_.swap(other.tail_);
            auto c = count_.exchange(other.count_.load(std::memory_order_relaxed), std::memory_order_relaxed);
            other.count_.exchange(c, std::memory_order_relaxed);
        }

    protected:
        NodePtr head_;
        NodePtr tail_;
        std::atomic<size_t> count_{0};
    };

} // namespace Atom::Event
