#include <vector>
#include <string>

struct SendBuffer
{
    std::vector<char> data;
    std::size_t offset = 0;

    SendBuffer(std::string buffer_data):
        data(buffer_data.begin(), buffer_data.end()) {};
};