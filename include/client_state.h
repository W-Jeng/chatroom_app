#pragma once

#include <iostream>
#include <memory>
#include <sstream>

class ChatSession;

class SessionState
{
public:
    virtual ~SessionState() = default;

    virtual void prompt(ChatSession& session) = 0;

    virtual void connect(ChatSession& session)
    {
        std::cout << "invalid action for current state \n";
    }

    virtual void join_room(ChatSession& session)
    {
        std::cout << "Invalid action for current state \n";
    }

    virtual void send_message(ChatSession& session, const std::string& msg)
    {
        std::cout << "Invalid action for current state \n";
    }
};

class ChatSession
{
public:
    ChatSession(std::unique_ptr<SessionState> initial_state):
        state(std::move(initial_state)) {};

    void set_state(std::unique_ptr<SessionState> new_state)
    {
        state = std::move(new_state);
    }

    void prompt()
    {
        state -> prompt(*this);
    }

    void connect()
    {
        state -> connect(*this);
    }

    void join_room()
    {
        state -> join_room(*this);
    }

    void send_message(const std::string& msg)
    {
        state -> send_message(*this, msg);
    }

private:
    std::unique_ptr<SessionState> state;
};

class JoinRoomState: public SessionState
{
public: 
    void prompt(ChatSession& session) override;
};

class DisconnectState: public SessionState 
{
public:
    void prompt(ChatSession& session) override
    {
        while (true)
        {
            std::cout << "Active State: Disconnected State\n";
            std::cout << "[Key 1] Join Room\n";
            std::cout << "[Key 2] End Application\n";
            std::string response;
            std::getline(std::cin, response);

            if (response != "1" && response != "2")
            {
                std::cout << "Invalid input. Please try again\n\n";
                continue;
            }

            session.set_state(std::unique_ptr<SessionState>(new JoinRoomState()));
            break;
        }
    }

    void connect(ChatSession& session) override
    {
        std::cout << "Connecting to server!\n";
    }
};

void JoinRoomState::prompt(ChatSession& session)
{
    while (true)
    {
        std::cout << "Active State: Join Room State\n";
        std::cout << "Please key in Room Number\n";
        std::cout << "Key in -1 to return to disconnect\n";
        std::string response;
        std::getline(std::cin, response);

        if (response == "-1")
        {
            std::cout << "Returning to Disconnect state\n\n";
            session.set_state(std::unique_ptr<SessionState>(new DisconnectState()));
            break;
        }
    }
}

