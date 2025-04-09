#include <string>
#include <vector>
#include <iostream>

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

    void receive(std::string client_msg) {
        // client_msg must be a complete message
    }

private:
    std::vector<Room> rooms;


};