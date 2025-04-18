#pragma once

#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <message_protocol.h>
#include <client_state.h>

constexpr int PORT = 8080;
constexpr int BUFFER_SIZE = 4096;

class Client
{
public:
    Client() {}

    void run()
    {
        ChatSession chat_session(std::make_unique<DisconnectState>());
        chat_session.prompt();


        fd_ = socket(AF_INET, SOCK_STREAM, 0);

        if (fd_ < 0)
        {
           std::cerr << "Client Socket creation failed \n";
           return;
        }

        // Define the server address
        sockaddr_in client_address{};
        client_address.sin_family = AF_INET;
        client_address.sin_port = htons(PORT);

        if (inet_pton(AF_INET, "127.0.0.1", &client_address.sin_addr) <= 0)
        {
            std::cerr << "Invalid address\n";
            return;
        }

        // Connect to server
        if (connect(fd_, (struct sockaddr*) &client_address, sizeof(client_address)) < 0)
        {
            std::cerr << "Connection failed!\n";
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
    }

    void stop()
    {
        if (fd_ > 0)
        {
            close(fd_);
        }

        std::cout << "Clean exit from client\n";
    }

private:
    int fd_ = -1;
};