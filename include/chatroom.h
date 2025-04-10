#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>

struct Room {
    std::string name;
    std::vector<int> users;
};

class Chatroom {
public:
    Chatroom() {
        std::cout << "helllo world from chatroom!\n";
        rooms.push_back(Room{"default"});
    }

    void receive(int fd, std::string client_msg) {
        messages[fd] += client_msg;
    }

private:
    std::vector<Room> rooms;
    std::unordered_map<int, std::string> messages;
};