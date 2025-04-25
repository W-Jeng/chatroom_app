#pragma once
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
#include <unordered_map>
#include <message_protocol.h>
#include <room_manager.h>
#include <send_buffer.h>
#include <queue>

constexpr int PORT = 8080;
constexpr int BUFFER_SIZE = 4096;
std::mutex client_sockets_mutex;

class Server 
{
public:
    Server(int max_clients):
        MAX_CLIENTS_(max_clients) {}

    void run() 
    {
        bool server_fd_created = create_server_socket();

        if (!server_fd_created) 
        {
            std::cerr << "Unable to properly setup server socket!\n";
            return;
        }

        std::cout << "Server socket (int): " << server_fd << "\n";

        if (!set_socket_not_blocking(server_fd)) 
        {
            return;
        }

        while (!stop_server) 
        {
            int client_fd = accept_connection();

            if (client_fd != -1 && !set_socket_not_blocking(client_fd)) 
            {
                return;
            }

            receive_message_and_process();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        shutdown_sockets();
    }

    void stop() 
    {
        stop_server = true;
    }

private:
    int server_fd;
    std::vector<std::unique_ptr<SocketHandle>> client_sockets;   
    std::unordered_map<int, std::string> buffer_msg;
    std::mutex mut;
    int MAX_CLIENTS_;
    std::atomic<bool> stop_server{false};
    RoomManager room_manager;

    void shutdown_sockets() 
    {
        std::cout << "\nShutdown Signal received, closing sockets and shutting down the server\n";
        std::lock_guard<std::mutex> lock(client_sockets_mutex);
        client_sockets.clear();

        if (server_fd != -1) {
            close(server_fd);
            std::cout << "Closed server socket: " << server_fd << "\n";
        }
    }

    bool create_server_socket() 
    {
        server_fd = socket(AF_INET, SOCK_STREAM, 0);

        if (server_fd == -1) 
        {
            std::cerr << "Socket creation failed\n";
            return false;
        }

        int opt = 1;

        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) 
        {
            perror("setsockopt(SO_REUSEADDR) failed");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in server_address{};
        server_address.sin_family = AF_INET;
        server_address.sin_addr.s_addr = INADDR_ANY;
        server_address.sin_port = htons(PORT);

        if (bind(server_fd, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) 
        {
            std::cerr << "Binding failed\n";
            return false;
        }

        if (listen(server_fd, MAX_CLIENTS_) == -1) 
        {
            std::cerr << "Listening failed\n";
            return false;
        }

        std::cout << "Server listening on port: " << PORT << "\n";
        return true;
    }

    void receive_message_and_process() 
    {
        std::vector<int> closed_connections;
        std::lock_guard<std::mutex> lck(mut);

        for (int i = 0; i < client_sockets.size(); ++i) 
        {
            char buffer[BUFFER_SIZE];
            ssize_t bytes = recv(client_sockets[i] -> get_fd(), buffer, sizeof(buffer), 0);

            if (bytes > 0) 
            {
                buffer_msg[client_sockets[i]->get_fd()] = std::string(buffer, bytes);
                std::cout << "Server123 has received: " << std::string(buffer, bytes) << "\n";
                on_message_update(client_sockets[i]->get_fd());
                memset(buffer, 0, sizeof(buffer));
            }
            else if (bytes == 0) 
            {
                closed_connections.push_back(i);
            }
            else 
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK) 
                {
                    continue;
                } 
                else if (errno == ECONNRESET) 
                {
                    closed_connections.push_back(i);
                } 
                else 
                {
                    perror("recv");
                    closed_connections.push_back(i);
                }
            }
        }

        if (closed_connections.size() != 0) 
        {
            std::reverse(closed_connections.begin(), closed_connections.end());

            for (int index: closed_connections) 
            {
                buffer_msg.erase(client_sockets[index]->get_fd());
                client_sockets.erase(client_sockets.begin()+index);
            }
        }
    }

    int accept_connection() 
    {
        struct sockaddr_in client_address{};
        socklen_t client_len = sizeof(client_address);

        int client_fd = accept(server_fd, (struct sockaddr*) &client_address, &client_len);

        if (client_fd != -1) 
        {
            std::lock_guard lck(mut);
            client_sockets.push_back(std::make_unique<SocketHandle>(client_fd));
            std::cout << "Client connected at socket (int): " << client_fd << "\n";
            std::cout << "Current Client Sockets size: " << client_sockets.size() << "\n";
            Message return_msg;
            return_msg.from_fd = server_fd;
            return_msg.to_fd = client_fd;
            return_msg.action = Action::Messaging;
            return_msg.data = "Server has received the connection!";
            return_msg.msg_len = return_msg.size();
            std::cout << return_msg.repr() << std::endl;
            send(client_fd, return_msg.repr().c_str(), return_msg.repr().size(), 0);
        }

        return client_fd;
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

    void on_message_update(int fd)
    {
        std::unique_ptr<Message> msg = MessageProtocol::decode(buffer_msg[fd]);

        if (msg)
        {
            std::cout << "\n\nOn Message Update: " << msg -> repr() << "\n";
            int msg_len = msg -> msg_len;
            buffer_msg[fd] = buffer_msg[fd].substr(msg_len, buffer_msg[fd].size()-msg_len);
            process_full_message(fd, std::move(msg));
        }
    }

    void process_full_message(int sent_from, std::unique_ptr<Message> msg)
    {
        bool success;

        switch (msg->action)
        {
            case Action::CreateRoom:
                room_manager.create_room(msg -> data);
                break;

            case Action::JoinRoom:
                success = room_manager.join_room(sent_from, msg -> data);

                if (success)
                {
                    std::cout << "User successfully joined the room\n";
                    Message return_msg;
                    return_msg.from_fd = server_fd;
                    return_msg.to_fd = sent_from;
                    return_msg.action = Action::JoinRoom;
                    return_msg.data = "SUCCESS";
                    return_msg.msg_len = return_msg.size();
                    std::cout << return_msg.repr() << std::endl;
                    send(sent_from, return_msg.repr().c_str(), return_msg.repr().size(), 0);
                }
                else
                {
                    std::cout << "Room does not exist or you haven't joined the room!\n";
                    Message return_msg;
                    return_msg.from_fd = server_fd;
                    return_msg.to_fd = sent_from;
                    return_msg.action = Action::JoinRoom;
                    return_msg.data = "FAILED";
                    return_msg.msg_len = return_msg.size();
                    std::cout << return_msg.repr() << std::endl;
                    send(sent_from, return_msg.repr().c_str(), return_msg.repr().size(), 0);
                }
                
                break;

            case Action::LeaveRoom:
                room_manager.leave_room(msg -> from_fd, msg -> data);
                break;

            case Action::Messaging:
                std::cout << "messaging action called\n";
                std::vector<Message> outgoing_messages = room_manager.on_messaging(server_fd, sent_from, msg -> data);
                std::cout << "outgoing msg size: " << outgoing_messages.size() << "\n";
                echo(outgoing_messages);
                break;
        }
    }

    void echo(const std::vector<Message>& outgoing_messages)
    {
        // in not blocking scenes, we need to understand that sometimes client isnt always recv, so we need to store it in a queue 
        std::queue<SendBuffer> send_q;

        for (Message msg: outgoing_messages)
        {
            std::cout << "\n \n From server raw sending: " << msg.repr() << "\n\n";
            send(msg.to_fd, msg.repr().c_str(), msg.repr().size(), 0);
            // send_q.emplace(msg);
        }

        while (!send_q.empty())
        {
            SendBuffer& buf = send_q.front();
            std::cout << "buffer data: " << buf.data.data() << "\n";
            ssize_t n = send(buf.underlying.to_fd, buf.data.data()+buf.offset, buf.data.size()-buf.offset, 0);
            std::cout << "sent n: " << n << "\n";

            if (n > 0)
            {
                buf.offset += static_cast<size_t>(n);

                if (buf.offset == buf.data.size())
                {
                    send_q.pop();
                }
                else
                {
                    continue;
                }
            }   
            else if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
            {
                continue;
            }
            else
            {
                perror("send failed");
                send_q.pop();
                continue;
            }
        }
    }
};
