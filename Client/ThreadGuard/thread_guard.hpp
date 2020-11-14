#pragma once

#include <thread>

class ThreadGuard
{
public:
    explicit ThreadGuard(std::thread& t_): t(t_) { }
    ~ThreadGuard()
    {
        if (t.joinable() == true)
            t.join();
    }
    //ThreadGuard(ThreadGuard const& other) = delete;
    //ThreadGuard& operator=(ThreadGuard const& other) = delete;
private:
    std::thread& t;
};
