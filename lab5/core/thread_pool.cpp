#include "thread_pool.h"

#include <iostream>
#include <chrono>

static std::mutex cout_mutex;

void Task::operator()() const
{
    std::this_thread::sleep_for(std::chrono::seconds(this->duration));
    std::lock_guard<std::mutex> lock(cout_mutex);
    std::cout << "task " << this->id << " Execution time: " << this->duration << " seconds" << std::endl;
}

ThreadPool::ThreadPool(int num_threads)
{
    thread_count = num_threads;
}


ThreadPool::~ThreadPool()
{
    stop();

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

        queue_length_sum += tasks.size();
        queue_samples++;
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
            auto wait_start = std::chrono::steady_clock::now();
            condition.wait(lock);
            auto wait_end = std::chrono::steady_clock::now();

            auto wait_duration = std::chrono::duration_cast<std::chrono::microseconds>(wait_end - wait_start).count();
            total_wait_time += wait_duration;
            wait_count++;
        }

        if(stopped)
        {
            return;
        }

        task = tasks.top();
        tasks.pop();

        lock.unlock();

        task();
        tasks_executed++;
        total_execution_time += task.duration;
    }
}

size_t ThreadPool::get_queue_length()
{
    std::lock_guard<std::mutex> lock(queue_mutex);
    return tasks.size();
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

void ThreadPool::print_stats()
{
    int executed = tasks_executed.load();
    int total_time = total_execution_time.load();
    double avg_queue = 0;
    double avg_wait = 0;

    if(executed == 0)
    {
        std::cout << "No tasks executed" << std::endl;
        return;
    }

    if(queue_samples > 0)
    {
        avg_queue = (double)queue_length_sum / queue_samples;
    }

    if(wait_count > 0)
    {
        avg_wait = (double)total_wait_time / wait_count / 1000.0;
    }


    std::cout << "Tasks executed: " << executed << std::endl;
    std::cout << "Average execution time: " << (double)total_time/executed << " seconds" << std::endl;
    std::cout << "Average queue length: "<< avg_queue << std::endl;
    std::cout << "Average worker wait time: " << avg_wait << " ms" << std::endl;
    std::cout << "Queue size at end: " << get_queue_length() << std::endl;
}
