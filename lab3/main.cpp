#include <iostream>
#include <thread>
#include <random>
#include "thread_pool.h"

Task task_generator()
{
    Task task;

    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dist(5, 10);

    static int id_counter = 0;

    task.id = id_counter++;
    task.duration = dist(gen);

    return task;
}

int main()
{
    ThreadPool pool(4);

    for(int i = 0; i < 10; i++)
    {
        Task task = task_generator();
        pool.add_task(task);
    }

    return 0;
}
