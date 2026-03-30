#include <iostream>
#include <WinSock2.h>
#include <vector>
#include <chrono>

#define port 8080

void print_matrix(std::vector<int>& matrix, int matrix_size)
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


bool receive_all(SOCKET socket, char* buffer, int totalBytes)
{
    int received = 0;

    while (received < totalBytes)
    {
        int bytes = recv(socket, buffer + received, totalBytes - received, 0);

        if (bytes == SOCKET_ERROR || bytes == 0)
        {
            return false;
        }

        received += bytes;
    }

    return true;
}


bool receive_matrix(SOCKET clientSocket, std::vector<int> &matrix, int &size, int &threads_num)
{
    // receiving size
    if (!receive_all(clientSocket, (char*) &size, sizeof(size)))
    {
        return false;
    }

    // receiving threads number
    if (!receive_all(clientSocket, (char*) &threads_num, sizeof(threads_num)))
    {
        return false;
    }

    // receiving matrix
    matrix.resize(size * size);
    if  (!receive_all(clientSocket, (char*) &matrix[0], matrix.size() * sizeof(int)))
    {
        return false;
    }

    return true;
}


int main()
{
    // Initialize Winsock
    WSAData wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "Error WSAStartup: " << WSAGetLastError() << std::endl;
        return 1;
    }

    // Server socket configuration
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET)
    {
        std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    //Bind the socket
    if (bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) != 0)
    {
        std::cerr << "Error binding socket: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, 5) != 0)
    {
        std::cerr << "Error listening on socket: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server listening on port " << port << std::endl;

    sockaddr_in clientAddress{};
    int clientSize = sizeof(clientAddress);

    SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddress, &clientSize);
    if (clientSocket == INVALID_SOCKET)
    {
        std::cerr << "Error accepting: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }


    // receiving matrix from client
    int size = 0;
    int threads_num = 0;
    std::vector<int> matrix;

    if (!receive_matrix(clientSocket, matrix, size, threads_num))
    {
        std::cerr << "Error receiving matrix: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Matrix received!" << std::endl;
    std::cout << "Size of matrix: " << size << std::endl;
    std::cout << "Threads number: " << threads_num << std::endl;

    print_matrix(matrix, size);

      // // Sending answer
    // const char* answer = "Hello client! I'm fine";
    // int bytesSent = send(clientSocket, answer, strlen(answer), 0);
    // if (bytesSent == SOCKET_ERROR)
    // {
    //     std::cerr << "Error sending message: " << WSAGetLastError() << std::endl;
    // }


    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
