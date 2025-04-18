#pragma once

#include <client_state.h>
#include <iostream>
#include <memory>

class DisconnectState;

class JoinRoomState: public SessionState
{
public: 
    void prompt(ChatSession& session) override
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
                session.set_state(std::make_unique<DisconnectState>());
                break;
            }
        }
    }
};


