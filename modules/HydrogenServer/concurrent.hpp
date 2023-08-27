#pragma once

#include <map>
#include <vector>

template <class M>
class ConcurrentSet
{
    unsigned long identifier = 1;
    std::map<unsigned long, M *> items;

public:
    void insert(M *item)
    {
        item->id = identifier++;
        items[item->id] = item;
        item->current = (ConcurrentSet<void> *)this;
    }

    void erase(M *item)
    {
        items.erase(item->id);
        item->id = 0;
        item->current = nullptr;
    }

    std::vector<unsigned long> ids() const
    {
        std::vector<unsigned long> result;
        for (auto item : items)
        {
            result.push_back(item.first);
        }
        return result;
    }

    M *operator[](unsigned long id) const
    {
        auto e = items.find(id);
        if (e == items.end())
        {
            return nullptr;
        }
        return e->second;
    }

    class iterator
    {
        friend class ConcurrentSet<M>;
        const ConcurrentSet<M> *parent;
        std::vector<unsigned long> ids;
        // Will be -1 when done
        long int pos = 0;

        void skip()
        {
            if (pos == -1)
                return;
            while (pos < (long int)ids.size() && !(*parent)[ids[pos]])
            {
                pos++;
            }
            if (pos == (long int)ids.size())
            {
                pos = -1;
            }
        }

    public:
        iterator(const ConcurrentSet<M> *parent) : parent(parent) {}

        bool operator!=(const iterator &o)
        {
            return pos != o.pos;
        }

        iterator &operator++()
        {
            if (pos != -1)
            {
                pos++;
                skip();
            }
            return *this;
        }

        M *operator*() const
        {
            return (*parent)[ids[pos]];
        }
    };

    iterator begin() const
    {
        iterator result(this);
        for (auto item : items)
        {
            result.ids.push_back(item.first);
        }
        result.skip();
        return result;
    }

    iterator end() const
    {
        iterator result(nullptr);
        result.pos = -1;
        return result;
    }
};

/* An object that can be put in a ConcurrentSet, and provide a heartbeat
 * to detect removal from ConcurrentSet
 */
class Collectable
{
    template <class P>
    friend class ConcurrentSet;
    unsigned long id = 0;
    const ConcurrentSet<void> *current;

    /* Keep the id */
    class HeartBeat
    {
        friend class Collectable;
        unsigned long id;
        const ConcurrentSet<void> *current;
        HeartBeat(unsigned long id, const ConcurrentSet<void> *current)
            : id(id), current(current) {}

    public:
        bool alive() const
        {
            return id != 0 && (*current)[id] != nullptr;
        }
    };

protected:
    /* heartbeat.alive will return true as long as this item has not changed collection.
     * Also detect deletion of the Collectable */
    HeartBeat heartBeat() const
    {
        return HeartBeat(id, current);
    }
};