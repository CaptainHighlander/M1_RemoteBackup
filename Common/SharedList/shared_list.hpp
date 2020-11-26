#pragma once

#include <list>
#include <mutex>

///This class allows to thread-safe use a genric list.
template <class T>
class SharedList
{
public:
    SharedList() { }

    [[nodiscard]] T front(void)
    {
        std::lock_guard<std::mutex> lg(mutex);
        return list.front();
    }

    void push_front(const T& t)
    {
        std::lock_guard<std::mutex> lg(mutex);
        list.push_front(t);
    }

    void pop_front(void)
    {
        std::lock_guard<std::mutex> lg(mutex);
        list.pop_front();
    }

    [[nodiscard]] T back(void)
    {
        std::lock_guard<std::mutex> lg(mutex);
        return list.back();
    }

    void push_back(const T& t)
    {
        std::lock_guard<std::mutex> lg(mutex);
        list.push_back(t);
    }

    void pop_back(void)
    {
        std::lock_guard<std::mutex> lg(mutex);
        list.pop_back();
    }

    [[nodiscard]] size_t size(void)
    {
        std::lock_guard<std::mutex> lg(mutex);
        return list.size();
    }

    [[nodiscard]] bool empty(void)
    {
        std::lock_guard<std::mutex> lg(mutex);
        return list.empty();
    }
private:
    T& operator=(const SharedList<T>& that);
    SharedList(const SharedList<T>& that);

    std::mutex mutex;
    std::list<T> list;
};
