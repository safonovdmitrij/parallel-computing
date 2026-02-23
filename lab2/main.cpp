#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <ctime>
#include <atomic>

constexpr size_t size = 5000000;
constexpr size_t threadsNum = 2;


void fillArray(std::vector<int>& array, size_t size)
{
    for(size_t i = 0; i < size; i++)
    {
        array[i] = rand() % (10000 - 10 + 1) + 10;
    }
}

void printArray(std::vector<int>& array, size_t size)
{
    for(size_t i = 0; i < size; i++)
    {
        std::cout << array[i] << " ";
    }
}

void processSegment(std::vector<int>& array, size_t start, size_t finish, std::atomic<int>& minimum, std::atomic<long long>& globalSum)
{
    long long localSum = 0;
    int localMinimum = INT_MAX;
    bool found = false;

    for(size_t i = start; i < finish; i++)
    {
        if(array[i] % 10 == 0)
        {
            found = true;
            localSum += array[i];

            if(array[i] < localMinimum)
            {
                localMinimum = array[i];
            }
        }
    }

    if(found)
    {
        long long currentSum = globalSum.load();
        while(!globalSum.compare_exchange_weak(currentSum, currentSum + localSum));

        int currentMinimum = minimum.load();

        while(localMinimum < currentMinimum && !minimum.compare_exchange_weak(currentMinimum, localMinimum))
        {
        }
    }
}


int main()
{
    srand(123);

    std::vector<int> array(size);
    std::atomic<long long> globalSum(0);
    std::atomic<int> minimum(INT_MAX);

    std::thread threads[threadsNum];
    size_t sectionSize = size / threadsNum;
    size_t start = 0;
    size_t finish = 0;

    fillArray(array, size);

    auto startTime = std::chrono::high_resolution_clock::now();

    for(size_t i = 0; i < threadsNum; i++)
    {
        start = i * sectionSize;
        finish = start + sectionSize;

        if(i == threadsNum - 1)
        {
            finish = size;
        }

        threads[i] = std::thread(processSegment, std::ref(array), start, finish, std::ref(minimum), std::ref(globalSum));
    }

    for(size_t i = 0; i < threadsNum; i++)
    {
        threads[i].join();
    }


    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

    std::cout << "\nGlobal Sum: " << globalSum.load() << std::endl;
    if(minimum.load() == INT_MAX)
    {
        std::cout << "No elements divisible by 10 found\n";
    }
    else
    {
        std::cout << "Minimum: " << minimum.load() << std::endl;
    }
    std::cout << "Total time: " << duration.count() << " microseconds\n";

    return 0;
}
