#pragma once
#include <unordered_map>
#include <chatroom.h>
#include <iostream>

struct OutgoingMessage
{
    int from_fd;
    int to_fd;
    int data;
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

        chatrooms[room_name] = Chatroom(room_name);
    }

    bool delete_room(int fd_user, const std::string& room_name)
    {
        /*
            delete_room operation can only be carried out by the person in it
        */
        if (!chatrooms.contains(room_name))
        {
            return false;
        }

        const std::unordered_set<int>& room_members = chatrooms[room_name].get_users_fd();

        if (!room_members.contains(fd_user)) 
        {
            return false;
        }

        chatrooms.erase(room_name);
        return true;
    }

    bool leave_room(int fd_user, const std::string& room_name)
    {
        if (!chatrooms.contains(room_name))
        {
            return false;
        }

        const std::unordered_set<int>& room_members = chatrooms[room_name].get_users_fd();

        if (!room_members.contains(fd_user))
        {
            return false;
        }

        chatrooms[room_name].erase(fd_user);
        return true;
    }

    

private:
    std::unordered_map<std::string, Chatroom> chatrooms;
};