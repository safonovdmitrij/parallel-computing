#include <iostream>
#include <vector>

const int size = 100;

void fill_array(std::vector<int>& array, int size)
{
    for(int i = 0; i < size; i++)
    {
        array[i] = rand() % 1000;
    }
}

void print_array(std::vector<int>& array, int size)
{
    for(int i = 0; i < size; i++)
    {
        std::cout << array[i] << " ";
    }
}

void processArray(std::vector<int>& array, int size, int& minimum, int& globalSum)
{
    for(int i = 0; i < size; i++)
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
    std::vector<int> array(size);
    int globalSum = 0;
    int minimum = 10000;

    fill_array(array, size);
    print_array(array, size);

    processArray(array, size, minimum, globalSum);

    std::cout << "\nGlobal Sum: " << globalSum << std::endl;
    if(minimum == 10000)
    {
        std::cout << "No elements divisible by 10 found\n";
    }
    else
    {
        std::cout << "Minimum: " << minimum << std::endl;
    }

    return 0;
}
