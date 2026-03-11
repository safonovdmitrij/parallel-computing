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

    int thread_count = 0;

    bool running = false;
    bool paused = false;
    bool stopped = false;

    void worker();

public:
    ThreadPool(int num_threads);
    ~ThreadPool();

    void add_task(const Task& task);

    void start();
    void pause();
    void resume();
    void stop();
};


#endif //THREAD_POOL_H
