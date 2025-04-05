#include <vector>
#include <iostream>
#include <csignal>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <mutex>
#include <algorithm>
#include <signal_handler.h>

constexpr int PORT = 8080;
constexpr int MAX_CLIENTS = 10;
constexpr int BUFFER_SIZE = 4096;
std::mutex client_sockets_mutex;


class Server {
public:
    void run()
    {
    }

private:
    std::vector<int> client_sockets;   

    static void shutdown_sockets(int signal)
    {
        std::cout << "\nShutdown Signal received, closing sockets and shutting down the server\n";
        std::lock_guard<std::mutex> lock(client_sockets_mutex);

        for (int client_socket: client_sockets)
        {
            close(client_socket);
        }


        std::exit(0);
    }
};