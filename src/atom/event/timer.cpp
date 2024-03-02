/*
 * timer.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-11

Description: Timer manager.

**************************************************/

#include "timer.hpp"
#include "eventloop.hpp"

#include <string.h>
#include <vector>

ATOM_NS_USING

ATOM_NS_BEGIN

int find_first_set(unsigned int b);
TICK_COUNT_TYPE get_tick_count_ms();
TICK_COUNT_TYPE calc_time_elapse_delta_ms(TICK_COUNT_TYPE now_tick,
                                          TICK_COUNT_TYPE &start_tick);

ATOM_NS_END

//////////////////////////////////////////////////////////////////////////
// Timer::Impl
Timer::Impl::Impl(TimerManager::Ptr mgr) : timer_mgr_(mgr) {}

Timer::Impl::~Impl() { cancel(); }

bool Timer::Impl::schedule(uint32_t delay_ms, Mode mode, TimerCallback cb) {
    auto mgr = timer_mgr_.lock();
    if (mgr) {
        return mgr->scheduleTimer(&timer_node_, delay_ms, mode, std::move(cb));
    }
    return false;
}

void Timer::Impl::cancel() {
    auto mgr = timer_mgr_.lock();
    if (mgr) {
        mgr->cancelTimer(&timer_node_);
    }
}

//////////////////////////////////////////////////////////////////////////
// TimerManager
TimerManager::TimerManager(EventLoop::Impl *loop) : loop_(loop) {
    memset(&tv0_bitmap_, 0, sizeof(tv0_bitmap_));
    for (int i = 0; i < TV_COUNT; ++i) {
        for (int j = 0; j < TIMER_VECTOR_SIZE; ++j) {
            list_init_head(&tv_[i][j]);
        }
    }
}

TimerManager::~TimerManager() {
    std::vector<TimerCallback> timer_cbs;  // hold timer cb temporarily
    std::lock_guard<std::mutex> g(mutex_);
    for (int i = 0; i < TV_COUNT && timer_count_ > 0; ++i) {
        for (int j = 0; j < TIMER_VECTOR_SIZE && timer_count_ > 0; ++j) {
            while (!list_empty(&tv_[i][j])) {
                auto *timer_node = tv_[i][j].next_;
                list_remove_node(timer_node);
                timer_cbs.emplace_back(timer_node->cancel());
                --timer_count_;
            }
        }
    }
}

bool TimerManager::scheduleTimer(TimerNode *timer_node, uint32_t delay_ms,
                                 Timer::Mode mode, TimerCallback cb) {
    if (isTimerPending(timer_node) && delay_ms == timer_node->delay_ms_) {
        return true;
    }
    TICK_COUNT_TYPE now_tick = get_tick_count_ms();
    bool need_notify = false;
    bool ret = false;
    {
        std::unique_lock<std::mutex> ul(mutex_);
        timer_node->cancelled_ = false;
        /*if (timer_node == running_node_ && !loop_->inSameThread()) {
            // timer is running, user may reschedule the timer in callback
            if(reschedule_node_ == timer_node) {
                reschedule_node_ = nullptr;
            }
            ul.unlock();
            running_mutex_.lock();
            if(running_node_ == timer_node) {
                running_node_ = nullptr;
            }
            running_mutex_.unlock();
            ul.lock();
            if (timer_node->cancelled_) {
                // timer is cancelled in callback
                return false;
            }
        }*/
        if (isTimerPending(timer_node)) {
            removeTimer(timer_node);
        }
        if (reschedule_node_ == timer_node) {
            reschedule_node_ = nullptr;
        }
        timer_node->start_tick_ = now_tick;
        timer_node->delay_ms_ = delay_ms;
        timer_node->repeating_ = mode == Timer::Mode::REPEATING;
        timer_node->cb_ = std::move(cb);

        ret = addTimer(timer_node, FROM_SCHEDULE);
        long diff = long(now_tick - last_tick_);
        if (last_remain_ms_ == -1 ||
            (diff >= 0 && delay_ms < last_remain_ms_ - diff)) {
            // need update poll wait time
            need_notify = !loop_->inSameThread();
        }
    }
    if (need_notify) {
        loop_->wakeup();
    }
    return ret;
}

void TimerManager::cancelTimer(TimerNode *timer_node) {
    if (timer_node->cancelled_) {
        return;
    }
    timer_node->cancelled_ = true;

    TimerCallback timer_cb;  // hold timer cb temporarily
    {
        std::unique_lock<std::mutex> ul(mutex_);
        if (running_node_ == timer_node && !loop_->inSameThread()) {
            if (reschedule_node_ == timer_node) {
                reschedule_node_ = nullptr;
            }
            ul.unlock();
            running_mutex_.lock();
            if (running_node_ == timer_node) {
                running_node_ = nullptr;
            }
            running_mutex_.unlock();
            ul.lock();
        }
        if (isTimerPending(timer_node)) {
            removeTimer(timer_node);
        }
        if (reschedule_node_ == timer_node) {
            reschedule_node_ = nullptr;
        }
        timer_cb = timer_node->cancel();
    }
}

void TimerManager::list_init_head(TimerNode *head) {
    head->next_ = head;
    head->prev_ = head;
}

void TimerManager::list_add_node(TimerNode *head, TimerNode *timer_node) {
    head->prev_->next_ = timer_node;
    timer_node->prev_ = head->prev_;
    timer_node->next_ = head;
    head->prev_ = timer_node;
}

void TimerManager::list_remove_node(TimerNode *timer_node) {
    timer_node->prev_->next_ = timer_node->next_;
    timer_node->next_->prev_ = timer_node->prev_;
    timer_node->resetNode();
}

void TimerManager::list_replace(TimerNode *old_head, TimerNode *new_head) {
    new_head->next_ = old_head->next_;
    new_head->next_->prev_ = new_head;
    new_head->prev_ = old_head->prev_;
    new_head->prev_->next_ = new_head;
    list_init_head(old_head);
}

void TimerManager::list_combine(TimerNode *from_head, TimerNode *to_head) {
    if (from_head->next_ == from_head) {
        return;
    }
    to_head->prev_->next_ = from_head->next_;
    from_head->next_->prev_ = to_head->prev_;
    from_head->prev_->next_ = to_head;
    to_head->prev_ = from_head->prev_;
    list_init_head(from_head);
}

bool TimerManager::list_empty(TimerNode *head) { return head->next_ == head; }

void TimerManager::set_tv0_bitmap(int idx) {
    unsigned char a = (unsigned char)(idx / (sizeof(tv0_bitmap_[0]) * 8));
    unsigned char b = (unsigned char)(idx % (sizeof(tv0_bitmap_[0]) * 8));
    tv0_bitmap_[a] |= 1 << b;
}

void TimerManager::clear_tv0_bitmap(int idx) {
    unsigned char a = (unsigned char)(idx / (sizeof(tv0_bitmap_[0]) * 8));
    unsigned char b = (unsigned char)(idx % (sizeof(tv0_bitmap_[0]) * 8));
    tv0_bitmap_[a] &= ~(1 << b);
}

int TimerManager::find_first_set_in_bitmap(int idx) {
    unsigned char a = (unsigned char)(idx / (sizeof(tv0_bitmap_[0]) * 8));
    unsigned char b = (unsigned char)(idx % (sizeof(tv0_bitmap_[0]) * 8));
    int pos = -1;
    pos = find_first_set(tv0_bitmap_[a] >> b);
    if (-1 == pos) {
        int i = a + 1;
        for (i &= 7; i != a; ++i, i &= 7) {
            pos = find_first_set(tv0_bitmap_[i]);
            if (-1 == pos) {
                continue;
            }
            if (i < a) {
                i += 8;
            }
            pos += (i - a - 1) * 32;
            pos += 32 - b;
            break;
        }
    }
    if (-1 == pos && b > 0) {
        unsigned int bits = (tv0_bitmap_[a] << (32 - b)) >> (32 - b);
        pos = find_first_set(bits);
        if (pos >= 0) {
            pos += 256 - b;
        }
    }
    return pos;
}

bool TimerManager::addTimer(TimerNode *timer_node, FROM from) {
    if (0 == timer_count_ && FROM_SCHEDULE == from) {
        last_tick_ = timer_node->start_tick_;
    }
    TICK_COUNT_TYPE fire_tick = timer_node->delay_ms_ + timer_node->start_tick_;
    if (fire_tick - last_tick_ > (((TICK_COUNT_TYPE)-1) >> 1))  // time backward
    {  // fire it right now
        fire_tick = last_tick_;
    }
    if (fire_tick == last_tick_) {
        ++fire_tick;  // = next_jiffies
    }
    TICK_COUNT_TYPE elapse_jiffies = fire_tick - last_tick_;
    TICK_COUNT_TYPE fire_jiffies = fire_tick;
    TimerNode *head;
    if (elapse_jiffies < TIMER_VECTOR_SIZE) {
        int i = fire_jiffies & TIMER_VECTOR_MASK;
        head = &tv_[0][i];
        set_tv0_bitmap(i);
        timer_node->tv_index_ = 0;
        timer_node->tl_index_ = i;
    } else if (elapse_jiffies < 1 << (2 * TIMER_VECTOR_BITS)) {
        int i = (fire_jiffies >> TIMER_VECTOR_BITS) & TIMER_VECTOR_MASK;
        head = &tv_[1][i];
        timer_node->tv_index_ = 1;
        timer_node->tl_index_ = i;
    } else if (elapse_jiffies < 1 << (3 * TIMER_VECTOR_BITS)) {
        int i = (fire_jiffies >> (2 * TIMER_VECTOR_BITS)) & TIMER_VECTOR_MASK;
        head = &tv_[2][i];
        timer_node->tv_index_ = 2;
        timer_node->tl_index_ = i;
    } else if (elapse_jiffies <= 0xFFFFFFFF) {
        int i = (fire_jiffies >> (3 * TIMER_VECTOR_BITS)) & TIMER_VECTOR_MASK;
        head = &tv_[3][i];
        timer_node->tv_index_ = 3;
        timer_node->tl_index_ = i;
    } else {  // don't support elapse larger than 0xffffffff
        // printf("add_timer, failed, elapse=%lu, start_tick=%lu,
        // last_tick=%lu\n", timer_node->elapse, timer_node->start_tick,
        // last_tick_);
        return false;
    }
    list_add_node(head, timer_node);
    if (FROM_SCHEDULE == from || FROM_RESCHEDULE == from) {
        ++timer_count_;
    }
    return true;
}

void TimerManager::removeTimer(TimerNode *timer_node) {
    if (0 == timer_node->tv_index_ && timer_node->next_ != timer_node &&
        timer_node->next_ == timer_node->prev_ &&
        timer_node->next_ == &tv_[0][timer_node->tl_index_]) {
        clear_tv0_bitmap(timer_node->tl_index_);
    }
    list_remove_node(timer_node);
    if (--timer_count_ == 0) {
        last_remain_ms_ = -1;
    }
}

int TimerManager::cascadeTimer(int tv_idx, int tl_idx) {
    TimerNode tmp_head;
    list_init_head(&tmp_head);
    list_replace(&tv_[tv_idx][tl_idx], &tmp_head);
    TimerNode *next_node = tmp_head.next_;
    TimerNode *tmp_node = nullptr;
    while (next_node != &tmp_head) {
        tmp_node = next_node;
        next_node = next_node->next_;
        addTimer(tmp_node, FROM_CASCADE);
    }

    return tl_idx;
}

#define JIFFIES_COMPARE(j1, j2) std::make_signed<TICK_COUNT_TYPE>::type(j1 - j2)
#define INDEX(N) \
    ((next_jiffies >> ((N + 1) * TIMER_VECTOR_BITS)) & TIMER_VECTOR_MASK)
int TimerManager::checkExpire(unsigned long *remain_ms) {
    if (0 == timer_count_) {
        last_remain_ms_ = -1;
        *remain_ms = last_remain_ms_;
        return 0;
    }
    TICK_COUNT_TYPE now_tick = get_tick_count_ms();
    TICK_COUNT_TYPE delta_tick =
        calc_time_elapse_delta_ms(now_tick, last_tick_);
    if (0 == delta_tick) {
        if (remain_ms) {
            if (last_remain_ms_ != -1 && last_remain_ms_ != 0) {
                *remain_ms = last_remain_ms_;
            } else {
                // calc remain time in ms
                mutex_.lock();
                int pos =
                    find_first_set_in_bitmap(now_tick & TIMER_VECTOR_MASK);
                mutex_.unlock();
                *remain_ms = -1 == pos ? 256 : pos;
                last_remain_ms_ = *remain_ms;
            }
        }
        return 0;
    }
    TICK_COUNT_TYPE cur_jiffies = now_tick;
    TICK_COUNT_TYPE next_jiffies = last_tick_ + 1;
    last_tick_ = now_tick;
    TICK_COUNT_TYPE last_tick = now_tick;
    TimerNode tmp_head;
    list_init_head(&tmp_head);
    std::vector<TimerCallback> timer_cbs;  // hold timer cb temporarily
    mutex_.lock();
    while (JIFFIES_COMPARE(cur_jiffies, next_jiffies) >= 0) {
        int idx = next_jiffies & TIMER_VECTOR_MASK;
#if 1
        int delta = 0;
        if (0 != idx && (delta = find_first_set_in_bitmap(idx)) != 0) {
            if (-1 == delta ||
                ((idx + delta) & TIMER_VECTOR_MASK) < idx) {  // run over 0
                delta =
                    TIMER_VECTOR_SIZE - idx;  // need cascade timer when index 0
            }
            idx += delta;
            idx &= TIMER_VECTOR_MASK;
            next_jiffies += delta;
            if (JIFFIES_COMPARE(next_jiffies, cur_jiffies) > 0) {
                next_jiffies = cur_jiffies + 1;
                break;
            }
        }
#endif
        ++next_jiffies;
        if (!idx && (!cascadeTimer(1, INDEX(0))) &&
            (!cascadeTimer(2, INDEX(1)))) {
            cascadeTimer(3, INDEX(2));
        }
        list_combine(&tv_[0][idx], &tmp_head);
        clear_tv0_bitmap(idx);
    }

    int count = 0;
    while (!list_empty(&tmp_head)) {
        reschedule_node_ = tmp_head.next_;
        running_node_ = reschedule_node_;
        list_remove_node(reschedule_node_);
        --timer_count_;
        mutex_.unlock();  // sync nodes in tmp_list with cancel_timer.

        running_mutex_.lock();
        if (running_node_) {
            (*running_node_)();
            running_node_ = nullptr;
            ++count;
        }
        running_mutex_.unlock();

        mutex_.lock();
        if (reschedule_node_) {
            if (reschedule_node_->repeating_ && !reschedule_node_->cancelled_) {
                reschedule_node_->start_tick_ = now_tick;
                addTimer(reschedule_node_, FROM_RESCHEDULE);
            } else {
                timer_cbs.emplace_back(reschedule_node_->cancel());
            }
            reschedule_node_ = nullptr;
        }
    }

    if (remain_ms) {
        // calc remain time in ms
        int pos = find_first_set_in_bitmap(cur_jiffies & TIMER_VECTOR_MASK);
        *remain_ms = -1 == pos ? 256 : pos;
    }

    mutex_.unlock();
    if (remain_ms) {  // revise the remain time
        now_tick = get_tick_count_ms();
        delta_tick = calc_time_elapse_delta_ms(now_tick, last_tick);
        if (*remain_ms <= delta_tick) {
            *remain_ms = 0;
        } else {
            *remain_ms -= (unsigned long)delta_tick;
        }
        last_remain_ms_ = *remain_ms;
    }
    return count;
}
