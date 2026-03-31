#include <iostream>
#include <WinSock2.h>
#include <vector>
#include <chrono>
#include <thread>
#include <atomic>

#define port 8080

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

// Processing matrix
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


int compute_sum(std::vector<int>& matrix, int row, int size)
{
    int sum = 0;
    for(int i = 0; i < size; i++)
    {
        if (i == row)
        {
            continue;
        }
        sum += matrix[row * size + i];
    }

    return sum;
}

void fill_diagonal(std::vector<int>& matrix, int start, int end, int size)
{
    for(int i = start; i < end; i++)
    {
        int sum = compute_sum(matrix, i, size);
        matrix[i * size + i] = sum;
    }
}


void parallel_compute(std::vector<int>& matrix, int size, int threads_num)
{
    std::vector<std::thread> threads(threads_num);

    int section_size = size / threads_num;
    int start = 0;
    int end = 0;

    for(int i = 0; i < threads_num; i++)
    {
        start = i * section_size;
        end = start + section_size;

        if (i == threads_num - 1)
        {
            end = size;
        }

        threads[i] = std::thread(fill_diagonal, std::ref(matrix), start, end, size);
    }

    for (int i = 0; i < threads_num; i++)
    {
        threads[i].join();
    }
}


// Application protocol
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

    // Response to client
    int response = htonl(1);

    if (!send_all(clientSocket, (char*) &response, sizeof(response)))
    {
        return false;
    }

    return true;
}


void compute_task(std::vector<int>& matrix, int size, int threads_num, std::atomic<Status>& current_status)
{
    std::this_thread::sleep_for(std::chrono::seconds(5));
    parallel_compute(matrix, size, threads_num);
    current_status = DONE;
}


bool handle_start(SOCKET clientSocket, std::vector<int> &matrix, int size, int threads_num, std::atomic<Status> &current_status)
{
    std::cout << "Computing started..." << std::endl;

    current_status = PROCESSING;

    std::thread compute_thread(compute_task, std::ref(matrix), size, threads_num, std::ref(current_status));
    compute_thread.detach();

    // sending answer to client (1 == Ok)
    int response = htonl(1);
    if (!send_all(clientSocket, (char*) &response, sizeof(response)))
    {
        return false;
    }

    return true;
}


bool handle_status(SOCKET clientSocket, std::atomic<Status> &current_status)
{
    std::cout << "Sending status..." << std::endl;

    int net_status = htonl(current_status.load());

    int bytes = send_all(clientSocket, (char*) &net_status, sizeof(net_status));

    if (bytes == SOCKET_ERROR || bytes == 0)
    {
        return false;
    }

    return true;
}


bool handle_result(SOCKET clientSocket, std::vector<int> &matrix)
{
    std::cout << "Sending result..." << std::endl;

    int element_count = matrix.size();
    int net_count = htonl(element_count);

    if (!send_all(clientSocket, (char*) &net_count, sizeof(net_count)))
    {
        return false;
    }

    // converting matrix to network order
    std::vector<int> net_matrix(element_count);

    for (int i = 0; i < element_count; i++)
    {
        net_matrix[i] = htonl(matrix[i]);
    }

    if (!send_all(clientSocket, (char*) &net_matrix[0], element_count * sizeof(int)))
    {
        return false;
    }

    return true;
}


bool process_client_commands(SOCKET clientSocket)
{
    int command = 0;

    int size = 0;
    int threads_num = 0;
    std::vector<int> matrix;

    std::atomic<Status> current_status = IDLE;

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
                if (!handle_start(clientSocket, matrix, size, threads_num, current_status))
                {
                    std::cerr << "Error handling START" << std::endl;
                    return false;
                }
                break;

            case STATUS:
                if (!handle_status(clientSocket, current_status))
                {
                    std::cerr << "Error handling STATUS" << std::endl;
                    return false;
                }
                break;

            case RESULT:
                if (!handle_result(clientSocket, matrix))
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


void client_handler(SOCKET clientSocket)
{
    process_client_commands(clientSocket);
    closesocket(clientSocket);
    std::cout << "Client disconnected" << std::endl;
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
    while (true)
    {
        SOCKET clientSocket = accept_client(serverSocket);
        if (clientSocket == INVALID_SOCKET)
        {
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }

        std::cout << "Client connected" << std::endl;

        std::thread client_thread(client_handler, clientSocket);

        client_thread.detach();
    }


    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
