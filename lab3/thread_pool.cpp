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
    for (int i = 0; i < num_threads; i++)
    {
        std::thread worker_thread(&ThreadPool::worker, this);
        workers.push_back(std::move(worker_thread));
    }
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

        while(tasks.empty())
        {
            condition.wait(lock);
        }

        task = tasks.top();
        tasks.pop();

        lock.unlock();

        task();
    }
}
