#include "thread_pool.h"

#include <iostream>
#include <chrono>



void Task::operator()() const
{
    std::this_thread::sleep_for(std::chrono::seconds(this->duration));
    std::cout << "task " << this->id << " Execution time: " << this->duration << " seconds" << std::endl;
}

ThreadPool::ThreadPool(int num_threads)
{
    thread_count = num_threads;
}


ThreadPool::~ThreadPool()
{
    for(int i = 0; i < workers.size(); i++)
    {
        workers[i].join();
    }
}


void ThreadPool::add_task(const Task& task)
{
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        tasks.push(task);
    }

    condition.notify_one();
}


void ThreadPool::worker()
{
    Task task;

    while (true)
    {
        std::unique_lock<std::mutex> lock(queue_mutex);

        while((tasks.empty() || paused) && !stopped)
        {
            condition.wait(lock);
        }

        if(stopped)
        {
            return;
        }

        task = tasks.top();
        tasks.pop();

        lock.unlock();

        task();
    }
}

void ThreadPool::start()
{
    std::lock_guard<std::mutex> lock(queue_mutex);

    if(running)
    {
        return;
    }

    running = true;
    paused = false;

    for(int i = 0; i < thread_count; i++)
    {
        workers.emplace_back(&ThreadPool::worker, this);
    }
}


void ThreadPool::pause()
{
    std::lock_guard<std::mutex> lock(queue_mutex);
    paused = true;
}


void ThreadPool::resume()
{
    std::lock_guard<std::mutex> lock(queue_mutex);
    paused = false;

    condition.notify_all();
}

void ThreadPool::stop()
{
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        stopped = true;
    }
    condition.notify_all();
}
