#pragma once
#include <memory>
#include <string>
#include <algorithm>
#include <iostream>

enum class Action 
{
    CreateRoom,
    JoinRoom,
    LeaveRoom,
    Messaging
};

inline std::string to_string(Action action) 
{
    switch (action) 
    {
        case Action::CreateRoom: return "CreateRoom";
        case Action::JoinRoom: return "JoinRoom";
        case Action::LeaveRoom: return "LeaveRoom";
        case Action::Messaging: return "Messaging";
        default: return "Unknown";
    }
}

Action decode_action(const std::string& action_str) 
{
    if (action_str == "CreateRoom") 
    {
        return Action::CreateRoom;
    }

    if (action_str == "JoinRoom") 
    {
        return Action::JoinRoom;
    }

    if (action_str == "LeaveRoom")
    {
        return Action::LeaveRoom;
    }

    if (action_str == "Messaging") 
    {
        return Action::Messaging;
    }

    throw std::invalid_argument("Unknown action: " + action_str);
};

struct Message
{
    int msg_len;
    int fd_id;
    Action action;
    std::string data;

    std::string repr() const 
    {
        return std::to_string(msg_len) + "/" + std::to_string(fd_id) + "/" +
            to_string(action) + "/" + data; 
    }
};

class MessageProtocol 
{
/*
    msg_len/id/action/msg
*/
public:
    static std::unique_ptr<Message> decode(const std::string& msg) 
    {
        if (msg.empty()) 
        {
            return nullptr;
        }

        std::string msg_len_str;
        std::string fd_id_str;
        std::string action_str;
        std::string data_str;

        auto end_of_msg_len_delim = std::find(msg.begin(), msg.end(), '/');

        if (end_of_msg_len_delim == msg.end())
        {
            return nullptr;
        }
        
        msg_len_str = msg.substr(0, end_of_msg_len_delim-msg.begin());
        int msg_len = std::stoi(msg_len_str);

        if (msg.size() < msg_len)
        {
            return nullptr;
        }

        auto end_of_fd_id_delim = std::find(end_of_msg_len_delim+1, msg.end(), '/');
        auto end_of_action_id_delim = std::find(end_of_fd_id_delim+1, msg.end(), '/');
        fd_id_str = msg.substr(static_cast<int>(end_of_msg_len_delim-msg.begin())+1, static_cast<int>(end_of_fd_id_delim-end_of_msg_len_delim)-1);
        action_str = msg.substr(static_cast<int>(end_of_fd_id_delim-msg.begin())+1, static_cast<int>(end_of_action_id_delim-end_of_fd_id_delim)-1); 
        data_str = msg.substr(static_cast<int>(end_of_action_id_delim-msg.begin())+1, msg_len-static_cast<int>(end_of_action_id_delim-msg.begin()));

        std::cout << "msg len_str: " << msg_len_str << "\n";
        std::cout << "fd id_str: " << fd_id_str << "\n";
        std::cout << "action_str: " << action_str << "\n";
        std::cout << "data_str: " << data_str << "\n";

        // std::cout << "msg str: " << msg_str << "\n";
        auto message = std::make_unique<Message>();
        message -> msg_len = msg_len;
        message -> fd_id = std::stoi(fd_id_str);
        message -> action = decode_action(action_str);
        message -> data = data_str;

        return message;
    }
};
