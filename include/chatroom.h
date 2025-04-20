#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <unordered_set>
#include <message_protocol.h>

class Chatroom 
{
public:
    Chatroom(std::string room_name):
        room_name_(room_name)
    {
        std::cout << "Chatroom initialized with name: " << room_name_ << "\n";
    }

    void add(int fd) 
    {
        users_fd.insert(fd);
    }

    void erase(int fd)
    {
        users_fd.erase(fd);
    }

    const std::unordered_set<int>& get_users_fd() const
    {
        return users_fd;
    }

private:
    std::string room_name_;
    std::unordered_set<int> users_fd;
};