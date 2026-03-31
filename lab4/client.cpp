#include <iostream>
#include <thread>
#include <WinSock2.h>
#include <vector>


#define port 8080
#define SERVER_IP "127.0.0.1"


const int matrix_size = 20;
const int threads_num = 3;

enum Command
{
    SEND_DATA = 1,
    START = 2,
    STATUS = 3,
    RESULT = 4,
    EXIT = 5
};

enum Status
{
    IDLE = 0,
    PROCESSING = 1,
    DONE = 2
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


// receiving answers
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


bool receive_status(SOCKET socket, int& status)
{
    int net_status;

    int bytes = recv(socket, (char*) &net_status, sizeof(net_status), 0);

    if (bytes == SOCKET_ERROR || bytes == 0)
    {
        return false;
    }

    status = ntohl(net_status);
    return true;
}


bool receive_result(SOCKET socket, std::vector<int>& matrix)
{
    int net_count;

    if (!receive_all(socket, (char*) &net_count, sizeof(net_count)))
    {
        return false;
    }

    int element_count = ntohl(net_count);
    matrix.resize(element_count);

    if (!receive_all(socket, (char*) &matrix[0], element_count * sizeof(int)))
    {
        return false;
    }

    for (int i = 0; i < element_count; i++)
    {
        matrix[i] = ntohl(matrix[i]);
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

    // waiting for response
    int net_response;

    if (!receive_all(socket, (char*) &net_response, sizeof(net_response)))
    {
        return false;
    }

    int response = ntohl(net_response);

    if (response != 1)
    {
        std::cerr << "Server failde to receive data" << std::endl;
        return false;
    }

    std::cout << "Server confirmed data received" << std::endl;

    return true;
}


bool start_computing(SOCKET socket)
{
    std::cout << "Sending START command..." << std::endl;

    if (!send_command(socket, START))
    {
        return false;
    }

    // waiting for response
    int net_response;

    if (!receive_all(socket, (char*) &net_response, sizeof(net_response)))
    {
        return false;
    }

    int response = ntohl(net_response);

    if (response != 1)
    {
        std::cerr << "Server did not start computation" << std::endl;
        return false;
    }

    std::cout << "Server started computation" << std::endl;

    return true;
}


bool get_status(SOCKET socket, int &status)
{
    std::cout << "Sending STATUS command..." << std::endl;

    if (!send_command(socket, STATUS))
    {
        return false;
    }

    if (!receive_status(socket, status))
    {
        return false;
    }

    return true;
}

bool get_result(SOCKET socket, std::vector<int>& matrix)
{
    std::cout << "Sending RESULT command..." << std::endl;

    if (!send_command(socket, RESULT))
    {
        return false;
    }

    if (!receive_result(socket, matrix))
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

    print_matrix(matrix);
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
    int status = IDLE;
    while (status != DONE)
    {
        if (!get_status(clientSocket, status))
        {
            std::cerr << "Error reading status" << std::endl;
        }

        std::cout << "Status: " << status << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // pause
    }


    // RESULT
    if (!get_result(clientSocket, matrix))
    {
        std::cerr << "Error reading result" << std::endl;
    }

    print_matrix(matrix);

    if (!send_exit(clientSocket))
    {
        std::cerr << "Error exiting" << std::endl;
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
