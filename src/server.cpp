#include <iostream>
#include <server.h>
#include <csignal>
#include <thread>
#include <chrono>
#include <message_protocol.h>

volatile std::sig_atomic_t g_stop = 0;

void handle_signal(int signal) 
{
    if (signal == SIGINT) 
    {
        g_stop = 1;
    }
}


int main() 
{
    std::unique_ptr<Message> msg = MessageProtocol::decode("19/3/4/JoinRoom/abcddd");

    if (msg) 
    {
        std::cout << "msg item: " << msg -> repr() << "\n";
    }

    std::cout << "Hello from server\n";
    std::signal(SIGINT, handle_signal);
    Server server{5};

    std::thread server_thread([&]()
    {
        server.run();
    });

    while (!g_stop) 
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    server.stop();
    server_thread.join();
    std::cout << "Clean shutdown\n";
    return 0;
}