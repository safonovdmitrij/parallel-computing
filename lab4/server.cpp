#include <iostream>
#include <WinSock2.h>
#include <vector>
#include <chrono>

#define port 8080

enum Command
{
    SEND_DATA = 1,
    START = 2,
    STATUS = 3,
    RESULT = 4,
    EXIT = 5
};


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


bool receive_command(SOCKET socket, int &command)
{
    int net_command;

    if (!receive_all(socket, (char*) &net_command, sizeof(net_command)))
    {
        return false;
    }

    command = ntohl(net_command);
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


// API
bool handle_send_data(SOCKET clientSocket, std::vector<int> &matrix, int &size, int &threads_num)
{
    if (!receive_matrix(clientSocket, matrix, size, threads_num))
    {
        return false;
    }

    std::cout << "Matrix received!" << std::endl;
    std::cout << "Size of matrix: " << size << std::endl;
    std::cout << "Threads number: " << threads_num << std::endl;

    print_matrix(matrix, size);
    return true;
}


bool handle_start()
{
    std::cout << "Computing started..." << std::endl;
    return true;
}


bool handle_status()
{
    std::cout << "Sending status..." << std::endl;
    return true;
}


bool handle_result()
{
    std::cout << "Sending result..." << std::endl;
    return true;
}


bool process_client_commands(SOCKET clientSocket)
{
    int command = 0;

    int size = 0;
    int threads_num = 0;
    std::vector<int> matrix;

    while (true)
    {
        if (!receive_command(clientSocket,command))
        {
            std::cerr << "Client disconnected or error" << std::endl;
            return false;
        }

        switch (command)
        {
            case SEND_DATA:
                if (!handle_send_data(clientSocket, matrix, size, threads_num))
                {
                    std::cerr << "Error handling SEND_DATA" << std::endl;
                    return false;
                }
                break;

            case START:
                if (!handle_start())
                {
                    std::cerr << "Error handling START" << std::endl;
                    return false;
                }
                break;

            case STATUS:
                if (!handle_status())
                {
                    std::cerr << "Error handling STATUS" << std::endl;
                    return false;
                }
                break;

            case RESULT:
                if (!handle_result())
                {
                    std::cerr << "Error handling RESULT" << std::endl;
                    return false;
                }
                break;

        case EXIT:
            std::cout << "Client requested disconnect" << std::endl;
            return true;

            default:
                {
                    std::cerr << "Unknown command: " << command << std::endl;
                    break;
                }
        }
    }
    return true;
}


// Connection methods
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


    // receiving
    process_client_commands(clientSocket);





    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
