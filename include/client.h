#pragma once

#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <message_protocol.h>
#include <client_state.h>
#include <fcntl.h>

constexpr int PORT = 8080;
constexpr int BUFFER_SIZE = 4096;

class Client
{
public:
    Client() {}

    void run()
    {
        fd_ = socket(AF_INET, SOCK_STREAM, 0);

        if (fd_ < 0)
        {
           std::cerr << "Client Socket creation failed \n";
           stop();
           return;
        }

        if (!set_socket_not_blocking(fd_))
        {
            std::cerr << "Failed in setting socket to not blocking\n";
            stop();
            return;
        }

        // Define the server address
        sockaddr_in client_address{};
        client_address.sin_family = AF_INET;
        client_address.sin_port = htons(PORT);

        if (inet_pton(AF_INET, "127.0.0.1", &client_address.sin_addr) <= 0)
        {
            std::cerr << "Invalid address\n";
            stop();
            return;
        }

        // Connect to server
        int connect_result = connect(fd_, (struct sockaddr*) &client_address, sizeof(client_address));

        if (connect_result < 0 && errno != EINPROGRESS)
        {
            std::cerr << "Connection failed!\n";
            stop();
            return;
        }

        std::cout << "Connected to server!\n";

        // When we connect, we are connect to the default room
        Message msg;
        msg.from_fd = fd_;
        msg.to_fd = fd_;
        msg.action = Action::Messaging;
        msg.data = "Connected to server! Client fd: [" + std::to_string(fd_) + "]\n";
        msg.msg_len = msg.size();
        send(fd_, msg.repr().data(), msg.repr().size(), 0);

        std::thread listen_thread([&]
        {
            listen_from_server();
        });

        ChatSession chat_session(std::make_unique<DisconnectState>(), fd_, app_stopped);
        chat_session.prompt();
        listen_thread.join();
    }

    void stop()
    {
        if (fd_ > 0)
        {
            close(fd_);
        }

        app_stopped = true;
        std::cout << "Clean exit from client\n";
    }

private:
    int fd_ = -1;
    bool app_stopped = false;

    void listen_from_server()
    {
        char buffer[BUFFER_SIZE] = {0};

        while (!app_stopped)
        {
            ssize_t bytes_received = read(fd_, buffer, BUFFER_SIZE);

            if (bytes_received > 0)
            {
                std::string msg_received(buffer, bytes_received);
                std::cout << "I have received msg from server: " << msg_received << "\n";
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    bool set_socket_not_blocking(int fd) 
    {
        int flags = fcntl(fd, F_GETFL, 0);

        if (flags == -1) 
        {
            std::cerr << "Failed to get socket flags!\n";
            return false;
        }

        flags |= O_NONBLOCK;

        if (fcntl(fd, F_SETFL, flags) == -1) 
        {
            std::cerr << "Failed to set socket to non-blocking!\n";
            return false;
        }

        return true;
    }
};