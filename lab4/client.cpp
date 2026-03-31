#include <iostream>
#include <WinSock2.h>
#include <vector>


#define port 8080
#define SERVER_IP "127.0.0.1"


const int matrix_size = 10;
const int threads_num = 3;

enum Command
{
    SEND_DATA = 1,
    START = 2,
    STATUS = 3,
    RESULT = 4,
    EXIT = 5
};


// matrix methods
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

// Application protocol methods
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

bool send_command(SOCKET socket, int command)
{
    int net_cmd = htonl(command);
    return send_all(socket, (char*) &net_cmd, sizeof(net_cmd));
}

bool send_matrix(SOCKET clientSocket, std::vector<int>& matrix, int size, int threads_num)
{
    // sending matrix size
    int net_size = htonl(size);
    if (!send_all(clientSocket, (char*) &net_size, sizeof(net_size)))
    {
        return false;
    }

    // sending threads number
    int net_threads_num = htonl(threads_num);
    if (!send_all(clientSocket, (char*) &net_threads_num, sizeof(net_threads_num)))
    {
        return false;
    }

    // sending matrix
    std::vector<int> net_matrix = matrix;
    for(int i = 0; i < net_matrix.size(); i++)
    {
        net_matrix[i] = htonl(net_matrix[i]);
    }
    if (!send_all(clientSocket, (char*) &net_matrix[0], net_matrix.size() * sizeof(int)))
    {
        return false;
    }

    return true;
}

// API
bool send_data(SOCKET socket, std::vector<int>& matrix, int size, int threads_num)
{
    std::cout << "Sending data..." << std::endl;

    if (!send_command(socket, SEND_DATA))
    {
        return false;
    }

    if (!send_matrix(socket, matrix, size, threads_num))
    {
        return false;
    }
    return true;
}


bool start_computing(SOCKET socket)
{
    std::cout << "Sending START command..." << std::endl;

    if (!send_command(socket, START))
    {
        return false;
    }
    return true;
}

bool get_status(SOCKET socket)
{
    std::cout << "Sending STATUS command..." << std::endl;

    if (!send_command(socket, STATUS))
    {
        return false;
    }
    return true;
}

bool get_result(SOCKET socket)
{
    std::cout << "Sending RESULT command..." << std::endl;

    if (!send_command(socket, RESULT))
    {
        return false;
    }
    return true;
}


bool send_exit(SOCKET socket)
{
    std::cout << "Sending EXIT command..." << std::endl;

    if (!send_command(socket, EXIT))
    {
        return false;
    }
    return true;
}

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



    // Sending data
    std::vector<int> matrix(matrix_size * matrix_size);
    fill_matrix(matrix);


    // SEND_DATA
    if (!send_data(clientSocket, matrix, matrix_size, threads_num))
    {
        std::cerr << "Error sending data" << std::endl;
    }

    // START
    if (!start_computing(clientSocket))
    {
        std::cerr << "Error starting computing" << std::endl;
    }

    // STATUS
    if (!get_status(clientSocket))
    {
        std::cerr << "Error reading status" << std::endl;
    }

    // RESULT
    if (!get_result(clientSocket))
    {
        std::cerr << "Error reading result" << std::endl;
    }

    if (!send_exit(clientSocket))
    {
        std::cerr << "Error exiting" << std::endl;
    }
    // Receiving result






    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
