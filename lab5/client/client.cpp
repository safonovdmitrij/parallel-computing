#include <iostream>
#include <thread>
#include <WinSock2.h>
#include <vector>


#define port 8080
#define SERVER_IP "127.0.0.1"


// connection methods
SOCKET setup_client()
{
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET)
    {
        return INVALID_SOCKET;
    }

    return clientSocket;
}

bool connect_to_server(SOCKET clientSocket)
{
    SOCKADDR_IN serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Connecting to server
    if (connect(clientSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) != 0)
    {
        return false;
    }

    return true;
}


int main()
{
    WSAData wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cout << "Error WSAStartup: " << WSAGetLastError() << std::endl;
        return 1;
    }

    SOCKET clientSocket = setup_client();
    if (clientSocket == INVALID_SOCKET)
    {
        std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    if (!connect_to_server(clientSocket))
    {
        std::cerr << "Error connecting to server: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }
    else
    {
        std::cout << "Connected to server" << std::endl;
    }


    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
