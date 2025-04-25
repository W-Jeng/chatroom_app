#pragma once
#include <unordered_map>
#include <chatroom.h>
#include <iostream>

struct OutgoingMessage
{
    int from_fd;
    int to_fd;
    std::string data;
};

class RoomManager
{
public:
    bool create_room(const std::string& room_name)
    {
        if (chatrooms.contains(room_name))
        {
            return false;
        }

        chatrooms.emplace(room_name, Chatroom(room_name));
        return true;
    }

    bool join_room(int fd_user, const std::string& room_name)
    {
        auto it = chatrooms.find(room_name);

        if (it == chatrooms.end())
        {
            return false;
        }

        const std::unordered_set<int>& room_members = it->second.get_users_fd();

        it->second.add(fd_user);
        std::cout << "Added fd: " << fd_user << " to room name: " << room_name << "\n\n";
        fd_to_room_mp[fd_user] = room_name;
        return true;
    }

    bool leave_room(int fd_user, const std::string& room_name)
    {
        auto it = chatrooms.find(room_name);

        if (it == chatrooms.end())
        {
            return false;
        }

        const std::unordered_set<int>& room_members = it->second.get_users_fd();

        if (!room_members.contains(fd_user)) 
        {
            return false;
        }

        it->second.erase(fd_user);
        fd_to_room_mp.erase(fd_user);
        return true;
    }

    std::vector<Message> on_messaging(int server_fd, int fd, const std::string& data)
    {
        const std::string& room_name = fd_to_room_mp[fd];
        auto it = chatrooms.find(room_name);

        if (it == chatrooms.end())
        {
            return {};
        }

        const std::unordered_set<int>& room_members = it->second.get_users_fd();

        if (!room_members.contains(fd))
        {
            return {};
        }

        std::vector<Message> messages;

        for (int member: room_members)
        {
            Message msg = Message();
            msg.from_fd = server_fd;
            msg.to_fd = member;
            msg.action = Action::Messaging;
            msg.data = data;
            msg.msg_len = msg.size();
            messages.push_back(msg);
        }

        return messages;
    }

private:
    std::unordered_map<std::string, Chatroom> chatrooms;
    std::unordered_map<int, std::string> fd_to_room_mp;
};