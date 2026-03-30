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
    int net_size = 0;
    if (!receive_all(clientSocket, (char*) &net_size, sizeof(net_size)))
    {
        return false;
    }
    size = ntohl(net_size);

    // receiving threads number
    int net_threads_num = 0;
    if (!receive_all(clientSocket, (char*) &net_threads_num, sizeof(net_threads_num)))
    {
        return false;
    }
    threads_num = ntohl(net_threads_num);

    // receiving matrix
    matrix.resize(size * size);
    if  (!receive_all(clientSocket, (char*) &matrix[0], matrix.size() * sizeof(int)))
    {
        return false;
    }

    for (int i = 0; i < matrix.size(); i++)
    {
        matrix[i] = ntohl(matrix[i]);
    }

    return true;
}


SOCKET setup_server()
{
    // Server socket configuration
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET)
    {
        return INVALID_SOCKET;
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    //Bind the socket
    if (bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) != 0)
    {
        closesocket(serverSocket);
        return INVALID_SOCKET;
    }

    if (listen(serverSocket, 5) != 0)
    {
        closesocket(serverSocket);
        return INVALID_SOCKET;
    }

    return serverSocket;
}


SOCKET accept_client(SOCKET serverSocket)
{
    sockaddr_in clientAddress{};
    int clientSize = sizeof(clientAddress);

    SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddress, &clientSize);
    if (clientSocket == INVALID_SOCKET)
    {
        return INVALID_SOCKET;
    }

    return clientSocket;
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
    SOCKET serverSocket = setup_server();
    if (serverSocket == INVALID_SOCKET)
    {
        WSACleanup();
        return 1;
    }
    else
    {
        std::cout << "Server listening on port " << port << std::endl;
    }

    // Client socket configuration
    SOCKET clientSocket = accept_client(serverSocket);
    if (clientSocket == INVALID_SOCKET)
    {
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    else
    {
        std::cout << "Client connected" << std::endl;
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
