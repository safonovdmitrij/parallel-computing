#include <iostream>
#include <WinSock2.h>

#define port 8080

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

    // receiving data from client
    char buffer[256]{};

    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

    if (bytesReceived > 0)
    {
        std::cout << "Received from client: ";
        std::cout.write(buffer, bytesReceived) << std::endl;
    }
    else if (bytesReceived == 0)
    {
        std::cout << "Client disconnected" << std::endl;
    }
    else
    {
        std::cerr << "Error receiving from client: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Sending answer
    const char* answer = "Hello client! I'm fine";
    int bytesSent = send(clientSocket, answer, strlen(answer), 0);
    if (bytesSent == SOCKET_ERROR)
    {
        std::cerr << "Error sending message: " << WSAGetLastError() << std::endl;
    }


    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
