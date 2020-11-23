#pragma once

#include <thread>

class ThreadGuard
{
public:
    explicit ThreadGuard(std::thread& _myThread);
    ~ThreadGuard(void);
    //ThreadGuard(const ThreadGuard& other) = delete;
    ThreadGuard& operator=(ThreadGuard const& other) = delete;
private:
    std::thread& myThread;
};
