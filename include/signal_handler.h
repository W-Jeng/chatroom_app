#include <csignal>
#include <iostream>

class SignalHandler{
public:
    SignalHandler(void (*handler)(int))
    {
        std::signal(SIGINT, handler);
    }
};