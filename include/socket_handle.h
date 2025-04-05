#include <unistd.h>
#include <iostream>

class SocketHandle{
public:
    SocketHandle(int fd):
        fd_(fd) 
    {
        std::cout << "Socket: " << fd_ << " initialized\n";
    }

    ~SocketHandle()
    {
        if (fd_ != -1)
        {
            close(fd_);
            std::cout << "Socket: " << fd_ << " closed\n";
        }
    }

private:
    int fd_;
};