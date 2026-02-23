#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <ctime>

constexpr size_t size = 5000000;

void fillArray(std::vector<int>& array, size_t size)
{
    for(int i = 0; i < size; i++)
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

void processArray(std::vector<int>& array, size_t size, int& minimum, long long& globalSum)
{
    for(size_t i = 0; i < size; i++)
    {
        if(array[i] % 10 == 0)
        {
            globalSum += array[i];

            if(array[i] < minimum)
            {
                minimum = array[i];
            }
        }
    }
}


int main()
{
    srand(123);
    std::vector<int> array(size);
    long long globalSum = 0;
    int minimum = INT_MAX;

    fillArray(array, size);


    auto startTime = std::chrono::high_resolution_clock::now();

    processArray(array, size, minimum, globalSum);

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

    std::cout << "\nGlobal Sum: " << globalSum << std::endl;
    if(minimum == INT_MAX)
    {
        std::cout << "No elements divisible by 10 found\n";
    }
    else
    {
        std::cout << "Minimum: " << minimum << std::endl;
    }
    std::cout << "Total time: " << duration.count() << " microseconds\n";

    return 0;
}
