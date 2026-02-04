#include <iostream>
#include <thread>
#include <chrono>

constexpr size_t n = 4;

int random_number_generator()
{
    return rand() % 10;
}

// заповнюємо матрицю рандомними числами, а головну діагональ -1
void fill_matrix(int matrix[][n])
{
    int col = 0;
    int row = 0;

    while(row < n)
    {
        while(col < n)
        {
            if(col == row) //головна діагональ
            {
                matrix[row][col] = -1;
                col++;
                continue;
            }
            matrix[row][col] = random_number_generator();
            col++;
        }
        col = 0;
        row++;
    }
}

// приймаємо на вхід вказівник на матрицю, розмір рядку, номер рядку суму якого потрібно порахувати.
int compute_sum(int matrix[][n], int row)
{
    int sum = 0;

    for(int i = 0; i < n; i++)
    {
        sum += matrix[row][i];
    }

    return sum;
}

void fill_diagonal(int matrix[][n])
{
    for(int i = 0; i < n; i++)
    {
        int sum = compute_sum(matrix, i);
        matrix[i][i] = sum + 1;
    }
}


void print_matrix(int matrix[][n])
{
    for(int i = 0; i < n; i++)
    {
        std::cout<<"[";

        for(int j = 0; j < n; j++)
        {
            if(j == n - 1) // щоб не було [1, 2, ..., n, ]
            {
                std::cout<<matrix[i][j];
                continue;
            }
            std::cout<<matrix[i][j]<<", ";
        }

        std::cout<<"]"<<std::endl;
    }

}

int main()
{
    int matrix[n][n];

    srand(time(0));

    fill_matrix(matrix);
    fill_diagonal(matrix);
    print_matrix(matrix);

    return 0;
}
