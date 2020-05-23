#ifndef __message_h__
#define __message_h__

namespace yegcpp
{
    enum function_codes : unsigned short
    {
        hello_world = 0x0001,
        broadcast = 0x0002,
        recv_broadcast = 0x0003,
        user_joined = 0x0011,
        user_left = 0x0022,
        goodbye = 0xFFFE,
        error = 0xFFFF
    };
};

#endif // __message_h__
