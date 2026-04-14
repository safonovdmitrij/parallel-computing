#include "thread_pool.h"

#include <iostream>
#include <chrono>
#include <fstream>

static std::mutex cout_mutex;

std::string read_request(SOCKET client)
{
    char buffer[1024];

    int bytes = recv(client, buffer, sizeof(buffer), 0);

    if(bytes <= 0)
    {
        return "";
    }

    buffer[bytes] = '\0';
    return std::string(buffer);
}

std::string parse_path(const std::string &request)
{
    size_t start = request.find("GET ") + 4;
    size_t end = request.find(" ", start);

    std::string path = request.substr(start, end - start);

    if (path == "/")
    {
        path = "/index.html";
    }

    return path;
}

bool load_file(std::string &path, std::string &content)
{
    std::ifstream file("static" + path);

    if(!file.is_open())
    {
        return false;
    }

    content.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    return true;
}

std::string build_response(const std::string& content)
{
    return "HTTP/1.1 200 OK\r\n"
            "Content-Length: " + std::to_string(content.size()) +
            "\r\n\r\n" + content;
}

std::string build_404()
{
    return "HTTP/1.1 404 Not Found\r\n"
           "Content-Length: 13\r\n\r\n404 Not Found";
}

void Task::operator()() const
{
    std::string request = read_request(client);

    if (request.empty())
    {
        closesocket(client);
        return;
    }

    std::string path = parse_path(request);

    std::string content;

    std::string response;

    if (load_file(path, content))
    {
        response = build_response(content);
    }
    else
    {
        response = build_404();
    }

    send(client, response.c_str(), response.size(), 0);

    closesocket(client);
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

        task = tasks.front();
        tasks.pop();

        lock.unlock();

        auto work_start = std::chrono::steady_clock::now();

        task();

        auto work_end = std::chrono::steady_clock::now();
        auto work_duration = std::chrono::duration_cast<std::chrono::microseconds>(work_end - work_start).count();
        total_execution_time += work_duration;

        tasks_executed++;
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
