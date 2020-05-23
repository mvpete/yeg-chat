#ifndef __async_read_message_h__
#define __async_read_message_h__

#include "read_message_op.h"
namespace yegcpp
{
    template <typename Socket, typename Handler>
    void async_read_message(Socket& s, Handler&& h)
    {
        read_message_op<Socket, Handler> op(s, std::forward<Handler>(h));
        op();
    }
};

#endif // __async_process_message_h__