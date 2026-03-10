#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

class Task
{
public:
    int id;
    int duration;

    void operator()() const;
};


class TaskCompare
{
public:
    bool operator()(const Task& a, const Task& b) const
    {
        return a.duration > b.duration;
    }
};


class ThreadPool
{
private:
    std::priority_queue<Task, std::vector<Task>, TaskCompare> tasks;
    std::vector<std::thread> workers;
    std::mutex queue_mutex;
    std::condition_variable condition;

public:
    ThreadPool(int num_threads);
    ~ThreadPool();

    void add_task(const Task& task);
    void worker();


};


#endif //THREAD_POOL_H
