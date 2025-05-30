#pragma once

#include <iostream>
#include <memory>
#include <sstream>
#include <message_protocol.h>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>

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
    std::queue<std::string> message_queue;
    std::condition_variable cond_var;
    std::mutex mut;
    std::unique_lock<std::mutex> lck{mut, std::defer_lock};

    ChatSession(bool& app_stoppped_):
        app_stopped(app_stoppped_)
    {};

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

    bool app_is_stopped()
    {
        return app_stopped;
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

    void receive_from_server()
    {
        if (message_queue.empty())
        {
            return;
        }

        std::string message_str = message_queue.front();
        message_queue.pop();
        std::unique_ptr<Message> msg = MessageProtocol::decode(message_str);

        if (msg)
        {
            std::cout << "Message received from server: " << msg -> repr() << "\n";
            state -> receive_from_server(*this, *msg);
        }
    }

private:
    std::unique_ptr<SessionState> state{nullptr};
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
    void receive_from_server(ChatSession& session, Message& msg) override;
};

class CreateRoomState: public SessionState
{
public:
    void prompt(ChatSession& session) override;
};

void JoinRoomState::prompt(ChatSession& session)
{
    std::string response;

    std::cout << "\n\nActive State: Join Room State\n";
    std::cout << "Please key in Room Number\n";
    std::cout << "Key in -1 to return to disconnect\n";
    std::cout << "Answer: ";
    std::getline(std::cin, response);

    if (session.app_is_stopped())
    {
        return;
    }

    if (response == "-1")
    {
        session.set_state(std::make_unique<DisconnectState>());
        return;
    } 

    while (!session.message_queue.empty())
    {
        session.message_queue.pop();
    }

    session.join_room(response);
    session.lck.lock();
    session.cond_var.wait(session.lck, [&]{return !session.message_queue.empty();});
    session.receive_from_server();
    // session.set_state(std::make_unique<OccupiedState>());
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

    send(client_fd, msg.repr().c_str(), msg.repr().size(), 0);
}

void JoinRoomState::receive_from_server(ChatSession& session, Message& msg)
{
    session.lck.unlock();

    if (msg.action == Action::JoinRoom && msg.data == "SUCCESS")
    {
        session.set_state(std::make_unique<OccupiedState>());
    }
    else if (msg.action == Action::JoinRoom && msg.data == "FAILED")
    {
        std::cout << "Joined failed, might be due to wrong room name\n";
        // need to unlock if not will cause deadlock
        session.prompt();
    }
    else
    {
        std::cout << "Unknown resolve by server: " << msg.repr() << "\n";
    }
}

void DisconnectState::prompt(ChatSession& session)
{
    std::string response;

    std::cout << "\n\nActive State: Disconnected State\n";
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
        session.set_state(std::make_unique<CreateRoomState>());
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
    std::string response;

    std::cout << "\n\nActive State: Occupied State\n";
    std::cout << "[Key -1] To leave room\n";
    std::cout << "Enter the message: ";
    std::getline(std::cin, response);

    if (response == "-1")
    {
        session.set_state(std::make_unique<DisconnectState>());
        return;
    }

    Message msg;
    msg.from_fd = session.get_client_fd();
    msg.to_fd = session.get_client_fd();
    msg.action = Action::Messaging;
    msg.data = response;
    msg.msg_len = msg.size();
    send(session.get_client_fd(), msg.repr().c_str(), msg.repr().size(), 0);
    session.receive_from_server();
    session.prompt();
}

void OccupiedState::receive_from_server(ChatSession& session, Message& msg)
{
    std::cout << "\n\nReceived a message from server: " << msg.data << std::endl;
}

void CreateRoomState::prompt(ChatSession& session)
{
    std::string response;

    std::cout << "\n\nActive State: Create Room State\n";
    std::cout << "[Key -1] To go back to disconnect state\n";
    std::cout << "Create a room with name: ";
    std::getline(std::cin, response);

    if (response == "-1")
    {
        session.set_state(std::make_unique<DisconnectState>());
        return;
    }

    Message msg;
    msg.from_fd = session.get_client_fd();
    msg.to_fd = session.get_client_fd();
    msg.action = Action::CreateRoom;
    msg.data = response;
    msg.msg_len = msg.size();
    send(session.get_client_fd(), msg.repr().c_str(), msg.repr().size(), 0);
    std::cout << "Message sent: " << msg.repr() << "\n";
    session.set_state(std::make_unique<DisconnectState>());
}
