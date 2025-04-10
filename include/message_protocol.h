#include <memory>
#include <string>
#include <algorithm>
#include <iostream>

enum class Action {
    CreateRoom,
    JoinRoom,
    Messaging
};

inline std::string to_string(Action action) {
    switch (action) {
        case Action::CreateRoom: return "CreateRoom";
        case Action::JoinRoom: return "JoinRoom";
        case Action::Messaging: return "Messaging";
        default: return "Unknown";
    }
}

Action decode_action(const std::string& action_str) {
    if (action_str == "CreateRoom") {
        return Action::CreateRoom;
    }

    if (action_str == "JoinRoom") {
        return Action::JoinRoom;
    }

    if (action_str == "Messaging") {
        return Action::Messaging;
    }

    throw std::invalid_argument("Unknown action: " + action_str);
};

class MessageProtocol {
/*
    id/action/msg_size/msg
*/
public:
    int fd_id;
    Action action;
    int msg_size;
    std::string message;

    std::string repr() const {
        return std::to_string(fd_id) + "/" + to_string(action) + "/" +
            std::to_string(msg_size) + "/" + message;
    }

    static std::unique_ptr<MessageProtocol> decode(const std::string& msg) {
        if (msg.empty()) {
            return nullptr;
        }

        std::string fd_id_str;
        std::string action_str;
        std::string msg_size_str;
        std::string msg_str;

        auto fd_id_str_it = std::find(msg.begin(), msg.end(), '/');

        if (fd_id_str_it == msg.end()) {
            return nullptr;
        }

        fd_id_str = msg.substr(0, static_cast<int>(fd_id_str_it-msg.begin()));
        std::cout << "fd_id_str: " << fd_id_str << "\n";

        auto action_str_it = std::find(fd_id_str_it+1, msg.end(), '/');

        if (action_str_it == msg.end()) {
            return nullptr;
        }

        action_str = msg.substr(static_cast<int>(fd_id_str_it-msg.begin())+1, static_cast<int>(action_str_it-(fd_id_str_it+1)));
        std::cout << "action_str: " << action_str << "\n";

        auto msg_size_str_it = std::find(action_str_it+1, msg.end(), '/');

        if (msg_size_str_it == msg.end()) {
            return nullptr;
        }

        msg_size_str = msg.substr(static_cast<int>(action_str_it-msg.begin())+1, static_cast<int>(msg_size_str_it-(action_str_it+1)));
        std::cout << "msg size str: " << msg_size_str << "\n";

        if (msg.end()-(msg_size_str_it+1) < std::stoi(msg_size_str)) {
            return nullptr;
        }

        msg_str = msg.substr(msg_size_str_it-msg.begin()+1, std::stoi(msg_size_str));
        std::cout << "msg str: " << msg_str << "\n";
        auto msg_protocol = std::make_unique<MessageProtocol>();
        (msg_protocol -> fd_id) = std::stoi(fd_id_str);
        (msg_protocol -> action) = decode_action(action_str);
        (msg_protocol -> msg_size) = std::stoi(msg_size_str);
        (msg_protocol -> message) = msg_str;

        return msg_protocol;
    }
};
