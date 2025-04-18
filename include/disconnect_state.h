#pragma once

#include <client_state.h>
#include <iostream>
#include <memory>

class JoinRoomState;

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

            session.set_state(std::make_unique<JoinRoomState>());
            break;
        }
    }

    void connect(ChatSession& session) override
    {
        std::cout << "Connecting to server!\n";
    }
};