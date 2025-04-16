#include <csignal>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <chrono>

constexpr int PORT = 8080;
constexpr int BUFFER_SIZE = 4096;

int sock = -1;

void handle_signal(int signal)
{
    if (sock == -1)
    {
        std::cout << "closing client sock \n";
        close(sock);
    }

    std::exit(0);
}

int main() {
    // Step 1: Create a socket
    std::signal(SIGINT, handle_signal);

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0) {
        std::cerr << "Socket creation failed\n";
        return 1;
    }

    // Step 2: Define the server address
    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
        std::cerr << "Invalid address\n";
        return 1;
    }

    // Step 3: Connect to the server
    if (connect(sock, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        std::cerr << "Connection failed\n";
        return 1;
    }

    std::cout << "Connected to server!\n";

    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    // Step 4: Send data
    // const char* message = "Hello, server!";
    // send(sock, message, strlen(message), 0);

    const char* message2 = "19/3/4/Messaging/abcdd";
    send(sock, message2, strlen(message2), 0);
    // Step 5: Receive the echo response
    char buffer[BUFFER_SIZE] = {0};

    while (true)
    {
        ssize_t bytes_received = read(sock, buffer, BUFFER_SIZE);
        if (bytes_received > 0) {
            std::cout << "Server replied: " << buffer << "\n";
        }

        memset(buffer, 0, BUFFER_SIZE);
    }

    // Step 6: Close socket
    close(sock);

    return 0;
}
