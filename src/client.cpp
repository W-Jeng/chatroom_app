#include <csignal>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <client.h>

volatile std::sig_atomic_t g_stop = 0;

void handle_signal(int signal)
{
    if (signal == SIGINT)
    {
        g_stop = 1;
    }
}

int main() {
    // Step 1: Create a socket
    std::signal(SIGINT, handle_signal);
    Client client;
    
    std::thread client_thread([&]()
    {
        client.run();
    });

    while (!g_stop)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    client.stop();
    client_thread.join();
    std::cout << "Clean shutdown from client\n";
    return 0;
}
