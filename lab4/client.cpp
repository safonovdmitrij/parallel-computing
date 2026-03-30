#include <iostream>
#include <WinSock2.h>
#include <vector>


#define port 8080
#define SERVER_IP "127.0.0.1"


const int matrix_size = 10;
const int threads_num = 3;


int random_number_generator()
{
    return rand() % 100;
}


void fill_matrix(std::vector<int>& matrix)
{
    int col = 0;
    int row = 0;

    while (row < matrix_size)
    {
        while (col < matrix_size)
        {
            if (col == row)
            {
                matrix[row * matrix_size + col] = -1;
                col++;
                continue;
            }
            matrix[row * matrix_size + col] = random_number_generator();
            col++;
        }
        col = 0;
        row++;
    }
}


void print_matrix(std::vector<int>& matrix)
{
    for (int i = 0; i < matrix_size; i++)
    {
        std::cout << "[";

        for (int j = 0; j < matrix_size; j++)
        {
            if (j == matrix_size - 1)
            {
                std::cout << matrix[i * matrix_size + j];
                continue;
            }
            std::cout << matrix[i * matrix_size + j] << ", ";
        }

        std::cout << "]" << std::endl;
    }
}


bool send_all(SOCKET socket, const char* data, int totalBytes)
{
    int sent = 0;

    while (sent < totalBytes)
    {
        int bytes = send(socket, data + sent, totalBytes - sent, 0);

        if (bytes == SOCKET_ERROR || bytes == 0)
        {
            return false;
        }

        sent += bytes;
    }
    return true;
}


bool send_matrix(SOCKET clientSocket, std::vector<int>& matrix, int size, int threads_num)
{
    // sending matrix size
    if (!send_all(clientSocket, (char*) &size, sizeof(size)))
    {
        return false;
    }

    // sending threads number
    if (!send_all(clientSocket, (char*) &threads_num, sizeof(threads_num)))
    {
        return false;
    }

    // sending matrix
    if (!send_all(clientSocket, (char*) &matrix[0], matrix.size() * sizeof(int)))
    {
        return false;
    }

    return true;
}


void start_computing(SOCKET socket)
{

}

void get_status(SOCKET socket)
{

}

void get_result(SOCKET socket)
{

}



int main()
{
    WSAData wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cout << "Error WSAStartup: " << WSAGetLastError() << std::endl;
        return 1;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET)
    {
        std::cerr << "Error creating socket" << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    SOCKADDR_IN serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Connecting to server
    if (connect(clientSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) != 0)
    {
        std::cerr << "Error connecting to server: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to server" << std::endl;

    // Sending data
    std::vector<int> matrix(matrix_size * matrix_size);
    fill_matrix(matrix);

    print_matrix(matrix);

    send_matrix(clientSocket, matrix, matrix_size, threads_num);


    // //Receiving answer
    // char buffer[256]{};
    // int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    // if (bytesReceived > 0)
    // {
    //     std::cout << "Received from server: ";
    //     std::cout.write(buffer, bytesReceived) << std::endl;
    // }
    // else if (bytesReceived == 0)
    // {
    //     std::cout << "Server disconnected" << std::endl;
    // }
    // else
    // {
    //     std::cerr << "Recv failed: " << WSAGetLastError() << std::endl;
    // }
    //
    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
