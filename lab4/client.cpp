#include <iostream>
#include <WinSock2.h>

#define port 8080
#define SERVER_IP "127.0.0.1"

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
    if (connect(clientSocket, (SOCKADDR*) &serverAddress, sizeof(serverAddress)) != 0)
    {
        std::cerr << "Error connecting to server: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to server" << std::endl;

    // Sending data
    const char* message = "Hello server! How are you?";

    int bytesSent = send(clientSocket, message, strlen(message), 0);
    if (bytesSent == SOCKET_ERROR)
    {
        std::cerr << "Error sending message: " << WSAGetLastError() << std::endl;
    }

    //Receiving answer
    char buffer[256]{};
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived > 0)
    {
        std::cout << "Received from server: ";
        std::cout.write(buffer, bytesReceived) << std::endl;
    }
    else if (bytesReceived == 0)
    {
        std::cout << "Server disconnected" << std::endl;
    }
    else
    {
        std::cerr << "Recv failed: " << WSAGetLastError() << std::endl;
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}