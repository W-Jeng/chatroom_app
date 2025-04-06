#include <iostream>
#include <server.h>

int main()
{
    std::cout << "Hello from server\n";
    Server server{5};
    server.run();
    return 0;
}