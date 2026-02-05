#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

constexpr size_t n = 10000;

int random_number_generator()
{
    return rand() % 100;
}

// заповнюємо матрицю рандомними числами, а головну діагональ -1
void fill_matrix(std::vector<int>& matrix)
{
    int col = 0;
    int row = 0;

    while(row < n)
    {
        while(col < n)
        {
            if(col == row)
            {
                matrix[row * n + col] = -1;
                col++;
                continue;
            }
            matrix[row * n + col] = random_number_generator();
            col++;
        }
        col = 0;
        row++;
    }
}


int compute_sum(std::vector<int>& matrix, int row)
{
    int sum = 0;

    for(int i = 0; i < n; i++)
    {
        sum += matrix[row * n + i];
    }

    return sum;
}

void fill_diagonal(std::vector<int>& matrix)
{
    for(int i = 0; i < n; i++)
    {
        int sum = compute_sum(matrix, i);
        matrix[i * n + i] = sum + 1;
    }
}


void print_matrix(std::vector<int>& matrix)
{
    for(int i = 0; i < n; i++)
    {
        std::cout<<"[";

        for(int j = 0; j < n; j++)
        {
            if(j == n - 1)
            {
                std::cout<<matrix[i * n + j];
                continue;
            }
            std::cout<<matrix[i * n + j]<<", ";
        }

        std::cout<<"]"<<std::endl;
    }

}

int main()
{
    std::vector<int> matrix(n * n);

    srand(time(0));

    fill_matrix(matrix);

    auto start = std::chrono::steady_clock::now();

    fill_diagonal(matrix);

    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout<<"Time: "<<duration.count()<<" us "<<std::endl;
    std::cout<<matrix[0]<<std::endl;
    return 0;
}
