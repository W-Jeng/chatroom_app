#include <vector>
#include <iostream>
#include <csignal>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <mutex>
#include <algorithm>
#include <socket_handle.h>
#include <memory>
#include <thread>
#include <chrono>
#include <atomic>
#include <fcntl.h>
#include <chatroom.h>

constexpr int PORT = 8080;
constexpr int BUFFER_SIZE = 4096;
std::mutex client_sockets_mutex;

class Server {
public:
    Server(int max_clients):
        MAX_CLIENTS_(max_clients) {}

    void run() {
        bool server_fd_created = create_server_socket();

        if (!server_fd_created) {
            std::cerr << "Unable to properly setup server socket!\n";
            return;
        }

        std::cout << "Server socket (int): " << server_fd << "\n";

        if (!set_socket_not_blocking(server_fd)) {
            return;
        }

        while (!stop_server) {
            int client_fd = accept_connection();

            if (client_fd != -1 && !set_socket_not_blocking(client_fd)) {
                return;
            }

            receive_message_and_process();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        shutdown_sockets();
    }

    void stop() {
        stop_server = true;
    }

private:
    int server_fd;
    std::vector<std::unique_ptr<SocketHandle>> client_sockets;   
    int MAX_CLIENTS_;
    std::atomic<bool> stop_server{false};
    Chatroom chatroom;

    void shutdown_sockets() {
        std::cout << "\nShutdown Signal received, closing sockets and shutting down the server\n";
        std::lock_guard<std::mutex> lock(client_sockets_mutex);
        client_sockets.clear();

        if (server_fd != -1) {
            close(server_fd);
            std::cout << "Closed server socket: " << server_fd << "\n";
        }
    }

    bool create_server_socket() {
        server_fd = socket(AF_INET, SOCK_STREAM, 0);

        if (server_fd == -1) {
            std::cerr << "Socket creation failed\n";
            return false;
        }

        struct sockaddr_in server_address{};
        server_address.sin_family = AF_INET;
        server_address.sin_addr.s_addr = INADDR_ANY;
        server_address.sin_port = htons(PORT);

        if (bind(server_fd, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
            std::cerr << "Binding failed\n";
            return false;
        }

        if (listen(server_fd, MAX_CLIENTS_) == -1) {
            std::cerr << "Listening failed\n";
            return false;
        }

        std::cout << "Server listening on port: " << PORT << "\n";
        return true;
    }

    void receive_message_and_process() {
        std::vector<int> closed_connections;

        for (int i = 0; i < client_sockets.size(); ++i) {
            char buffer[BUFFER_SIZE];
            ssize_t bytes = recv(client_sockets[i] -> get_fd(), buffer, sizeof(buffer), 0);
            
            if (bytes > 0) {
                std::cout << "Received buffer item -> bytes: " << bytes << ", message: " << std::string(buffer, bytes) << "\n";
                chatroom.receive(client_sockets[i]->get_fd(), std::string(buffer, bytes));
                memset(buffer, 0, sizeof(buffer));
            } else if (bytes == 0) {
                closed_connections.push_back(i);
            } else {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    return;
                } else if (errno == ECONNRESET) {
                    closed_connections.push_back(i);
                } else {
                    perror("recv");
                    closed_connections.push_back(i);
                }
            }
        }

        if (closed_connections.size() != 0) {
            std::reverse(closed_connections.begin(), closed_connections.end());

            for (int index: closed_connections) {
                client_sockets.erase(client_sockets.begin()+index);
            }
        }
    }

    int accept_connection() {
        struct sockaddr_in client_address{};
        socklen_t client_len = sizeof(client_address);

        int client_fd = accept(server_fd, (struct sockaddr*) &client_address, &client_len);

        if (client_fd != -1) {
            client_sockets.push_back(std::make_unique<SocketHandle>(client_fd));
            std::cout << "Client connected at socket (int): " << client_fd << "\n";
        }

        return client_fd;
    }

    bool set_socket_not_blocking(int fd) {
        int flags = fcntl(fd, F_GETFL, 0);

        if (flags == -1) {
            std::cerr << "Failed to get socket flags!\n";
            return false;
        }

        flags |= O_NONBLOCK;

        if (fcntl(fd, F_SETFL, flags) == -1) {
            std::cerr << "Failed to set socket to non-blocking!\n";
            return false;
        }

        return true;
    }
};
