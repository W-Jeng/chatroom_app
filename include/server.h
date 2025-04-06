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
#include <socket_handle.h>
#include <memory>
#include <thread>
#include <chrono>
#include <atomic>
#include <fcntl.h>

constexpr int PORT = 8080;
constexpr int BUFFER_SIZE = 4096;
std::mutex client_sockets_mutex;
std::mutex shutdown_mutex;
std::atomic<bool> shutdown_requested = false;

class Server {
public:
    Server(int max_clients):
        MAX_CLIENTS_(max_clients) {}

    void run()
    {
        bool server_socket_created = create_server_socket();

        if (!server_socket_created)
        {
            std::cerr << "Unable to properly setup server socket!\n";
            return;
        }

        std::cout << "Server socket (int): " << server_socket << "\n";

        std::signal(SIGINT, [](int)
        {
            std::cout << "Interrupt called!\n";
            shutdown_requested = true;
        });

        std::thread connectivity_thread([&]() {
            while (!shutdown_requested)
            {
                int flags = fcntl(server_socket, F_GETFL, 0);

                if (flags == -1)
                {
                    std::cerr << "Failed to get socket flags!\n";
                    break;
                }

                flags |= O_NONBLOCK;

                if (fcntl(server_socket, F_SETFL, flags) == -1)
                {
                    std::cerr << "Failed to set socket to non-blocking!\n";
                    break;
                }

                struct sockaddr_in client_address{};
                socklen_t client_len = sizeof(client_address);

                int client_socket = accept(server_socket, (struct sockaddr*) &client_address, &client_len);

                if (client_socket == -1)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    continue;
                }

                std::cout << "Client connected at socket (int): " << client_socket << "\n";
            }
        });


        connectivity_thread.join();
        shutdown_sockets();
    }

private:
    int server_socket;
    std::vector<std::unique_ptr<SocketHandle>> client_sockets;   
    int MAX_CLIENTS_;

    void shutdown_sockets()
    {
        std::cout << "\nShutdown Signal received, closing sockets and shutting down the server\n";
        std::lock_guard<std::mutex> lock(client_sockets_mutex);
        client_sockets.clear();

        if (server_socket != -1)
        {
            close(server_socket);
            std::cout << "Closed server socket: " << server_socket << "\n";
        }
    }

    bool create_server_socket()
    {
        server_socket = socket(AF_INET, SOCK_STREAM, 0);

        if (server_socket == -1)
        {
            std::cerr << "Socket creation failed\n";
            return false;
        }

        struct sockaddr_in server_address{};
        server_address.sin_family = AF_INET;
        server_address.sin_addr.s_addr = INADDR_ANY;
        server_address.sin_port = htons(PORT);

        if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1)
        {
            std::cerr << "Binding failed\n";
            return false;
        }

        if (listen(server_socket, MAX_CLIENTS_) == -1)
        {
            std::cerr << "Listening failed\n";
            return false;
        }

        std::cout << "Server listening on port: " << PORT << "\n";
        return true;
    }
};
