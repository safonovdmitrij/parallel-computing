#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <WinSock2.h>

class Task
{
public:
    SOCKET client;

    void operator()() const;
};


class ThreadPool
{
private:
    std::queue<Task> tasks;
    std::vector<std::thread> workers;
    std::mutex queue_mutex;
    std::condition_variable condition;

    // metrics
    std::atomic<int> tasks_executed = 0;
    std::atomic<int> total_execution_time = 0;
    std::atomic<long long> total_wait_time = 0;
    std::atomic<int> wait_count = 0;
    std::atomic<int> queue_samples = 0;
    std::atomic<long long> queue_length_sum = 0;

    int thread_count = 0;

    // flags
    bool running = false;
    bool paused = false;
    bool stopped = false;

    void worker();

public:
    ThreadPool(int num_threads);
    ~ThreadPool();

    void add_task(const Task& task);
    size_t get_queue_length();

    void start();
    void pause();
    void resume();
    void stop();

    void print_stats();
};


#endif //THREAD_POOL_H
