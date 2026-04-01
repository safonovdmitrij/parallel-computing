import socket
import struct
import time
import random

# Config
HOST = "127.0.0.1"
PORT = 8080

# Commands
SEND_DATA = 1
START = 2
STATUS = 3
RESULT = 4
EXIT = 5

# Status
IDLE = 0
PROCESSING = 1
DONE = 2

def send_int(sock, value):
    """Send int (4 bytes, network order)"""
    sock.sendall(struct.pack('!i', value))


def receive_all(sock, n):
    """Receive exactly n bytes"""
    data = b''
    while len(data) < n:
        packet = sock.recv(n - len(data))
        if not packet:
            raise Exception("Connection closed")
        data += packet
    return data


def receive_int(sock):
    """Receive int (4 bytes)"""
    data = receive_all(sock, 4)
    return struct.unpack('!i', data)[0]


# Matrix
def generate_matrix(size):
    """Generate matrix with -1 on diagonal"""
    matrix = []

    for i in range(size):
        for j in range(size):
            if i == j:
                matrix.append(-1)
            else:
                matrix.append(random.randint(1, 10))

    return matrix


def print_matrix(matrix, size):
    """Print matrix"""
    for i in range(size):
        row = matrix[i * size:(i + 1) * size]
        print(row)


def send_matrix(sock, matrix, size, threads):
    """Send matrix to server"""
    send_int(sock, size)
    send_int(sock, threads)

    for value in matrix:
        send_int(sock, value)


def receive_matrix(sock):
    """Receive matrix from server"""
    element_count = receive_int(sock)

    matrix = []
    for _ in range(element_count):
        matrix.append(receive_int(sock))

    return matrix


# API
def send_data(sock, matrix, size, threads):
    print("Sending matrix...")

    send_int(sock, SEND_DATA)
    send_matrix(sock, matrix, size, threads)

    response = receive_int(sock)
    print("SEND_DATA response:", response)


def start_computing(sock):
    print("Sending START command...")

    send_int(sock, START)

    response = receive_int(sock)
    print("START response:", response)


def wait_for_result(sock):
    print("Waiting for result...")

    status = IDLE

    while status != DONE:
        send_int(sock, STATUS)
        status = receive_int(sock)

        print("STATUS:", status)
        time.sleep(1)


def get_result(sock):
    print("Requesting result...")

    send_int(sock, RESULT)
    return receive_matrix(sock)


def disconnect(sock):
    print("Sending EXIT command...")
    send_int(sock, EXIT)


def main():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((HOST, PORT))

    print("Connected to server\n")

    size = 10
    threads = 3

    matrix = generate_matrix(size)

    print("Generated matrix:")
    print_matrix(matrix, size)
    print()

    send_data(sock, matrix, size, threads)

    start_computing(sock)

    wait_for_result(sock)

    result = get_result(sock)

    print("\nResult matrix:")
    print_matrix(result, size)

    disconnect(sock)

    sock.close()
    print("\nDisconnected")


if __name__ == "__main__":
    main()