#include <iostream>
#include <thread>
#include <random>
#include <atomic>
#include "thread_pool.h"

std::atomic<bool> stop_generators = false;

Task task_generator()
{
    Task task;

    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dist(5, 10);

    static std::atomic<int> id_counter = 0;

    task.id = id_counter++;
    task.duration = dist(gen);

    return task;
}


void generator(ThreadPool& pool)
{
    while (!stop_generators)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        Task task = task_generator();
        pool.add_task(task);
    }
}


void run_test(ThreadPool& pool, int seconds)
{
    std::cout << "Starting automatic test for " << seconds << " seconds..." << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(seconds));

    stop_generators = true;
    pool.stop();

    std::cout << "Results of automatic testing: " << std::endl;
    pool.print_stats();
}



int main()
{
    std::string command;
    ThreadPool pool(4);
    pool.start();

    const size_t generators_count = 2;

    const int test_duration = 20;

    std::vector<std::thread> generators;

    // generating tasks from few threads
    for(int i = 0; i < generators_count; i++)
    {
        generators.emplace_back(generator, std::ref(pool));
    }

    // commands input
    while(true)
    {
        std::cin >> command;

        if(command == "pause")
        {
            pool.pause();
        }

        if(command == "resume")
        {
            pool.resume();
        }

        if(command == "stop")
        {
            pool.stop();
        }

        if(command == "stats")
        {
            pool.print_stats();
        }

        if(command == "test")
        {
            run_test(pool, test_duration);
        }


        if(command == "exit")
        {
            stop_generators = true;
            pool.stop();
            break;
        }

    }

    for(int i = 0; i < generators_count; i++)
    {
        generators[i].join();
    }

    return 0;
}
