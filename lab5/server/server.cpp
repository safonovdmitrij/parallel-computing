#include <iostream>
#include <WinSock2.h>
#include <vector>
#include <chrono>
#include <thread>
#include <atomic>

#include "thread_pool.h"

#define port 8080

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

    std::cout << "Server listening on port " << port << std::endl;

    ThreadPool pool(4);
    pool.start();

    while (true)
    {
        SOCKET client = accept_client(serverSocket);

        if(client == INVALID_SOCKET)
        {
            std::cerr << "Accept failed" << WSAGetLastError << std::endl;
            continue;
        }
        else
        {
            Task task;
            task.client = client;

            pool.add_task(task);
        }
    }


    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
