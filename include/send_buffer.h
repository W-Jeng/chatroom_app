#include <vector>
#include <string>
#include <message_protocol.h>

struct SendBuffer
{
    std::vector<char> data;
    Message underlying;
    std::size_t offset = 0;

    SendBuffer(Message message):
        underlying(message)
    {
        std::string repr = message.repr();
        data.assign(repr.begin(), repr.end());
    }
};