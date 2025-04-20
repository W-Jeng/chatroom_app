#pragma once

#include <iostream>
#include <memory>
#include <sstream>
#include <message_protocol.h>

class ChatSession;

class SessionState
{
public:
    virtual ~SessionState() = default;

    virtual void prompt(ChatSession& session) = 0;

    virtual void connect(ChatSession& session)
    {
        std::cout << "Invalid action [connect] for current state \n";
    }

    virtual void join_room(ChatSession& session, const std::string& room_name)
    {
        std::cout << "Invalid action [join_room] for current state \n";
    }

    virtual void send_message(ChatSession& session, const std::string& msg)
    {
        std::cout << "Invalid action [send_message] for current state \n";
    }

    virtual void receive_from_server(ChatSession& session, Message& msg)
    {
        std::cout << "Invalid action [receive_from_server] for current state \n";
    }
};

class ChatSession
{
public:
    ChatSession(bool& app_stoppped_):
        app_stopped(app_stoppped_),
        state(nullptr) {};

    bool valid_state()
    {
        return (state != nullptr);
    }

    void set_state(std::unique_ptr<SessionState> new_state)
    {
        state = std::move(new_state);
        prompt();
    }

    void set_fd(int fd)
    {
        client_fd = fd;
    }

    void prompt()
    {
        if (app_stopped)
        {
            return;
        }
        
        state -> prompt(*this);
    }

    void connect()
    {
        state -> connect(*this);
    }

    void join_room(const std::string& room_name)
    {
        state -> join_room(*this, room_name);
    }

    void send_message(const std::string& msg)
    {
        state -> send_message(*this, msg);
    }

    int get_client_fd()
    {
        return client_fd;
    }

    void receive_from_server(const std::string& message)
    {
        std::unique_ptr<Message> msg = MessageProtocol::decode(message);

        if (msg)
        {
            std::cout << "the message repr: " << msg -> repr() << "\n";
            state -> receive_from_server(*this, *msg);
        }
    }

private:
    std::unique_ptr<SessionState> state;
    int client_fd;
    bool& app_stopped;
};

class JoinRoomState: public SessionState
{
public: 
    void prompt(ChatSession& session) override;
    void join_room(ChatSession& session, const std::string& room_name) override;
    void receive_from_server(ChatSession& session, Message& msg) override;
};

class DisconnectState: public SessionState 
{
public:
    void prompt(ChatSession& session) override;
};

class OccupiedState: public SessionState
{
// Joined a room state
public:
    void prompt(ChatSession& session) override;
};

void JoinRoomState::prompt(ChatSession& session)
{
    std::string response;

    std::cout << "\nActive State: Join Room State\n";
    std::cout << "Please key in Room Number\n";
    std::cout << "Key in -1 to return to disconnect\n";
    std::cout << "Answer: ";
    std::getline(std::cin, response);

    if (response == "-1")
    {
        session.set_state(std::make_unique<DisconnectState>());
    } 

    session.join_room(response);
    session.prompt();
};

void JoinRoomState::join_room(ChatSession& session, const std::string& room_name)
{
    int client_fd = session.get_client_fd();
    Message msg;
    msg.from_fd = client_fd;
    msg.to_fd = client_fd;
    msg.action = Action::JoinRoom;
    msg.data = room_name;
    msg.msg_len = msg.size();

    send(client_fd, msg.repr().data(), msg.repr().size(), 0);
}

void JoinRoomState::receive_from_server(ChatSession& session, Message& msg)
{
    if (msg.action == Action::JoinRoom && msg.data == "APPROVE")
    {
        session.set_state(std::make_unique<OccupiedState>());
    }
    else
    {
        std::cout << "Unknown resolve by server: " << msg.repr() << "\n";
    }
}

void DisconnectState::prompt(ChatSession& session)
{
    std::string response;

    std::cout << "\nActive State: Disconnected State\n";
    std::cout << "[Key 1] Join Room\n";
    std::cout << "[Key 2] Create Room\n";
    std::cout << "[Key -1] End Application\n";
    std::cout << "Answer: ";
    std::getline(std::cin, response);

    if (response == "1")
    {
        session.set_state(std::make_unique<JoinRoomState>());
    }
    else if (response == "2")
    {
        // TODO
        // session.set_state
        return;
    }
    else if (response == "-1")
    {
        std::cout << "Application ended. Press CTRL+C to exit to prompt\n";
        return;
    }
    else
    {
        std::cout << "Invalid input. Please try again\n\n";
        session.prompt();
    }
};

void OccupiedState::prompt(ChatSession& session)
{
    return;
}